/*
* SMITC.cpp:
 *
 * Author(s):
 *   Muktikanta Sa   <muktikanta.sa@gmail.com>
 *   
 * Description:
 *   
 * Copyright (c) 2018.
 * last Updated: 23/01/2018
 *
*/

#include<iostream>
#include<float.h>
#include<stdint.h>
#include<stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <ctime>        // std::time
#include <random>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
#include <atomic>
#include<list>


using namespace std;
#ifndef LLIST_H_ 
#define LLIST_H_


#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
#endif
#define MAX 100000

int d;
int SUTC[MAX];
int PREC[MAX];
typedef struct{
    int     secs;
    int     usecs;
}TIME_DIFF;

TIME_DIFF * my_difftime (struct timeval * start, struct timeval * end){
	TIME_DIFF * diff = (TIME_DIFF *) malloc ( sizeof (TIME_DIFF) );
 	if (start->tv_sec == end->tv_sec){
        	diff->secs = 0;
        	diff->usecs = end->tv_usec - start->tv_usec;
    	}
   	else{
        	diff->usecs = 1000000 - start->tv_usec;
        	diff->secs = end->tv_sec - (start->tv_sec + 1);
        	diff->usecs += end->tv_usec;
        	if (diff->usecs >= 1000000){
        	    diff->usecs -= 1000000;
	            diff->secs += 1;
	        }
	}
        return diff;
}


typedef struct Enode{
	int val; // data
	atomic<bool> marked;
	pthread_mutex_t lock;
	atomic<struct Enode *>enext; // pointer to the next adjancy list
}elist_t;

typedef struct TCnode{
	int val; // data
	atomic<bool> marked;
	pthread_mutex_t lock;
	atomic<struct TCnode *>tcnext; // pointer to the next adjancy list
}tclist_t;

typedef struct Vnode{
	int val; // data
	atomic<bool> marked;
	pthread_mutex_t lock;
	atomic<struct Enode *>enext;// pointer to the elist
	atomic<struct TCnode *>tcnext; // pointer to the tclist
	atomic<struct Vnode *>vnext; // pointer to the vlist
}vlist_t;

class Node{
           vlist_t *VHead, *VTail;
 public:
        Node(){
                VHead = (vlist_t*) malloc(sizeof(vlist_t));
                VHead ->val = INT_MIN;
                pthread_mutex_init(&VHead->lock, NULL);           
                VHead ->marked.store(false,memory_order_seq_cst);     
                VHead ->enext.store(NULL,memory_order_seq_cst);
                VHead ->tcnext.store(NULL,memory_order_seq_cst);
                VHead ->vnext.store(NULL,memory_order_seq_cst);
                VTail = (vlist_t*) malloc(sizeof(vlist_t));
                VTail ->val = INT_MAX;
                pthread_mutex_init(&VTail->lock, NULL);           
                VTail ->marked.store(false,memory_order_seq_cst);     
                VTail ->enext.store(NULL,memory_order_seq_cst);
                VTail ->tcnext.store(NULL,memory_order_seq_cst);
                VTail ->vnext.store(NULL,memory_order_seq_cst);
                VHead->vnext.store(VTail,memory_order_seq_cst);
           }
         elist_t* createE(int key){
                elist_t * temp = (elist_t*) malloc(sizeof(elist_t));
                temp ->val = key;
                pthread_mutex_init(&temp->lock, NULL);           
                temp ->marked.store(false,memory_order_seq_cst);     
                temp ->enext.store(NULL,memory_order_seq_cst);
                return temp;
        }  
         tclist_t* createTC(int key){
                tclist_t * temp = (tclist_t*) malloc(sizeof(tclist_t));
                temp ->val = key;
                pthread_mutex_init(&temp->lock, NULL);           
                temp ->marked.store(false,memory_order_seq_cst);     
                temp ->tcnext.store(NULL,memory_order_seq_cst);
                return temp;
        }  
        vlist_t* createV(int key){
                elist_t * EHead = createE(INT_MIN);
                elist_t * ETail = createE(INT_MAX);
                EHead->enext .store(ETail,memory_order_seq_cst); 
                tclist_t * TCHead = createTC(INT_MIN);
                tclist_t * TCTail = createTC(INT_MAX);
                tclist_t * newtc = createTC(key);
                newtc->tcnext .store(TCTail,memory_order_seq_cst); 
                TCHead->tcnext .store(newtc,memory_order_seq_cst); 
                vlist_t* newv = (vlist_t*) malloc(sizeof(vlist_t));
                newv->val = key;
                pthread_mutex_init(&newv->lock, NULL);           
                newv ->marked.store(false,memory_order_seq_cst);     
                newv->enext .store(EHead,memory_order_seq_cst);
                newv->vnext .store(NULL,memory_order_seq_cst);
                newv->tcnext .store(TCHead,memory_order_seq_cst); 
                return newv;
        }
          /*validating TC, pred and curr are not marked and pred->next = curr*/
        bool ValidateTC(tclist_t *pred, tclist_t *curr){
                if((pred->marked.load(memory_order_seq_cst) == false) && (curr->marked.load(memory_order_seq_cst) == false) && (pred->tcnext.load(memory_order_seq_cst) == curr))
                        return true;
                else
                        return false; 
               }
        /*validating V node, pred and curr are not marked and pred->vnext = curr*/               
        bool ValidateV(vlist_t *pred, vlist_t *curr){
                if((pred->marked.load(memory_order_seq_cst) == false) && (curr->marked.load(memory_order_seq_cst) == false) && (pred->vnext.load(memory_order_seq_cst) == curr))
                        return true;
                else
                        return false; 
               }
        /*validating E node, pred and curr are not marked and pred->enext = curr*/               
        bool ValidateE(elist_t *pred, elist_t *curr){
                if((pred->marked.load(memory_order_seq_cst) == false) && (curr->marked.load(memory_order_seq_cst) == false) && (pred->enext.load(memory_order_seq_cst) == curr))
                        return true;
                else
                        return false; 
               }  
        /*finds the key present in the CC, returns the reference of that cc , wait-free*/
        void locateE(elist_t* Ehead, elist_t ** pred, elist_t ** curr, int key){
                (* pred) = Ehead; 
                (* curr) = (*pred)->enext.load(memory_order_seq_cst); 
                while( (* curr)->enext.load(memory_order_seq_cst) != NULL && (* curr)->val < key){ //curr->enext != NULL &&
                        (*pred) = (* curr);
                        (* curr) = (* curr)->enext.load(memory_order_seq_cst); 
                       }
                  pthread_mutex_lock(&((*pred))->lock);
                 pthread_mutex_lock(&((* curr))->lock);    
                  
                 if(ValidateE((*pred), (* curr))){
                        
                        return;
                }
                else{
                         pthread_mutex_unlock(&(* curr)->lock);
                        pthread_mutex_unlock(&(*pred)->lock); 
                 return;
                }
                 }
        void locateV(vlist_t ** pred, vlist_t ** curr, int key){
                 
                  (* pred) = VHead; 
                 (* curr) =  (* pred)->vnext.load(memory_order_seq_cst); 
                while( (* curr)->vnext.load(memory_order_seq_cst) != NULL && (* curr)->val < key){ //curr->enext != NULL &&
                         (* pred) = (* curr);
                        (* curr) = (* curr)->vnext.load(memory_order_seq_cst); 
                       }
                pthread_mutex_lock(& (* pred)->lock);
                 pthread_mutex_lock(&(* curr)->lock); 
                
                 if(ValidateV( (* pred), (* curr))){
                       
                        return;
                }
                else{
                        pthread_mutex_unlock(&(* curr)->lock);
                         pthread_mutex_unlock(& (* pred)->lock); 
                         return;
                }
              }  
        void locateTC(tclist_t* TChead, tclist_t ** pred, tclist_t ** curr, int key){
                 
                 (* pred) = TChead; 
                (* curr) =  (* pred)->tcnext.load(memory_order_seq_cst); 
                while( (* curr)->tcnext.load(memory_order_seq_cst) != NULL && (* curr)->val < key){ //curr->enext != NULL &&
                         (* pred) = (* curr);
                        (* curr) = (* curr)->tcnext.load(memory_order_seq_cst); 
                       }
                pthread_mutex_lock(& (* pred)->lock);
                 pthread_mutex_lock(&(* curr)->lock); 
                
                 if(ValidateTC( (* pred), (* curr))){
                        
                        return;
                }
                else{
                        pthread_mutex_unlock(&(* curr)->lock);
                         pthread_mutex_unlock(& (* pred)->lock); 
                         return;
                }                
             }  
         bool ContainsE(elist_t *Ehead, elist_t ** n, int key){
                elist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr;
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return true;       // found it, return sutcess, true
                }
                else{
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return false; // key not present
                    }    
            }             
         bool ContainsV(vlist_t ** n, int key){
                vlist_t* pred, *curr;
                locateV(&pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        
                        return true;       // found it, return sutcess, true
                }
                else{
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return false; // key not present
                    }    
        }
        bool ContainsTC(tclist_t *TChead, tclist_t ** n, int key){
                tclist_t* pred, *curr;
                locateTC(TChead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return true;       // found it, return sutcess, true
                }
                else{
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return false; // key not present
                    }  
        }
        
         bool WFContainsE(elist_t *Ehead, elist_t ** n, int key){
                elist_t* pred = Ehead;
                elist_t* curr = pred->enext.load(memory_order_seq_cst); 
                while( curr->enext.load(memory_order_seq_cst) != NULL && curr->val < key){ //curr->enext != NULL &&
                        pred = curr;
                        curr = curr->enext.load(memory_order_seq_cst); 
                       }
                if((curr->val == key) && (curr->marked.load(memory_order_seq_cst) == false)){
                        (*n) = curr;
                        return true;       // found it, return sutcess, true
                }
                else{
                        return false; // key not present
                    }    
            }             
         bool WFContainsV(vlist_t ** n, int key){
                vlist_t* pred = VHead;
                vlist_t* curr = pred->vnext.load(memory_order_seq_cst); 
                while( curr->vnext.load(memory_order_seq_cst) != NULL && curr->val < key){ //curr->enext != NULL &&
                        pred = curr;
                        curr = curr->vnext.load(memory_order_seq_cst); 
                       }
                if((curr->val == key) && (curr->marked.load(memory_order_seq_cst) == false)){
                        (*n) = curr;
                        return true;       // found it, return sutcess, true
                }
                else{
                        return false; // key not present
                    }    
                
        }
        bool WFContainsTC(tclist_t *TChead, tclist_t ** n, int key){
                tclist_t* pred = TChead;
                tclist_t* curr = pred->tcnext.load(memory_order_seq_cst); 
                while( curr->tcnext.load(memory_order_seq_cst) != NULL && curr->val < key){ //curr->enext != NULL &&
                        pred = curr;
                        curr = curr->tcnext.load(memory_order_seq_cst); 
                       }
                if((curr->val == key) && (curr->marked.load(memory_order_seq_cst) == false)){
                        (*n) = curr;
                        return true;       // found it, return sutcess, true
                }
                else{
                        return false; // key not present
                    }    
                
        }
        
        
        bool AddE(elist_t *Ehead, int key){
                elist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                elist_t *newe = createE(key);
                if(curr->val != key){
                        newe ->enext.store(curr,memory_order_seq_cst);  
                        pred ->enext.store(newe,memory_order_seq_cst); // adding in the list, LP
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock);  
                        return true;
                }
                else{
                   pthread_mutex_unlock(&curr->lock);
                   pthread_mutex_unlock(&pred->lock);                       
                 return false;
              }             
             }  
         bool AddTC(tclist_t *TChead, int key){
                tclist_t* pred, *curr;
                locateTC(TChead, &pred, &curr, key);
                tclist_t *newtc = createTC(key);
                if(curr->val != key){
                        newtc ->tcnext.store(curr,memory_order_seq_cst);  
                        pred ->tcnext.store(newtc,memory_order_seq_cst); // adding in the list, LP
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock);  
                        return true;
                }
                else{
                   pthread_mutex_unlock(&curr->lock);
                   pthread_mutex_unlock(&pred->lock);                       
                 return false;
              }             
          }  
           bool AddV(int key){
                vlist_t* pred, *curr;
                locateV(&pred, &curr, key);
                vlist_t *newv = createV(key);
                if(curr->val != key){
                        newv ->vnext.store(curr,memory_order_seq_cst);  
                        pred ->vnext.store(newv,memory_order_seq_cst); // adding in the list, LP
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock);  
                        return true;
                }
                else{
                   pthread_mutex_unlock(&curr->lock);
                   pthread_mutex_unlock(&pred->lock);                       
                 return false;
              }             
             }
         bool RemoveE(elist_t *Ehead, int key){
                elist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        curr->marked.store(true, memory_order_seq_cst); // logical delete, LP
                        pred ->enext.store(curr->enext.load(memory_order_seq_cst),memory_order_seq_cst); // Physcial delete
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        curr->enext.store(NULL,memory_order_seq_cst);
                        free(curr);
                        return true;
                }
                else{
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return false;
                  }      
          }  
          bool RemoveTC(tclist_t *TChead, int key){
                tclist_t* pred, *curr;
                locateTC(TChead, &pred, &curr, key);
                if(curr->val == key){
                        curr->marked.store(true, memory_order_seq_cst); // logical delete, LP
                        pred ->tcnext.store(curr->tcnext.load(memory_order_seq_cst),memory_order_seq_cst); // Physcial delete
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        curr->tcnext.store(NULL,memory_order_seq_cst);
                        free(curr);
                        return true;
                }
                else{
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return false;
                  }      
          }  
          
         bool RemoveV(int key, vlist_t **n1, vlist_t **n2){
                vlist_t* pred, *curr;
                locateV(&pred, &curr, key);
                (*n1) = pred; (*n2) = curr;
                if(curr->val == key){
                        curr->marked.store(true, memory_order_seq_cst); // logical delete, LP
                        pred ->vnext.store(curr->vnext.load(memory_order_seq_cst),memory_order_seq_cst); // Physcial delete
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        curr->vnext.store(NULL,memory_order_seq_cst);
                        free(curr);
                        return true;
                }
                else{
                        pthread_mutex_unlock(&curr->lock);
                        pthread_mutex_unlock(&pred->lock); 
                        return false;
                  }      
               }
              
        
     
       // add each reachable vertices in vHead to tcHead 
           void  remVTCfromU(vlist_t *u, vlist_t *v){
               //tclist_t *pred1 = tcHead, *curr1 = pred->tcnext;
               tclist_t *tempu = u->tcnext.load(memory_order_seq_cst) , *tempv = v->tcnext.load(memory_order_seq_cst) ->tcnext.load(memory_order_seq_cst) ;
               //cout<<"R-tempu & tempv:"<<tempu->val<<" "<<tempv->val<<endl;
               while(tempv->tcnext.load(memory_order_seq_cst)  != NULL){
                 RemoveTC(tempu, tempv->val);
                 tempv = tempv->tcnext.load(memory_order_seq_cst) ;
               }
           }   
        /*DFS of the backward traversal*/      
         void RDFSBW(elist_t * slHead, vlist_t *v){
             vlist_t * curr, *pred;//used for iteration of each vertex in findVSameCC
             
             //cout<<"R-slHead->val & v->val:"<<slHead->val<<" "<<v->val<<endl;
             //cin>>z;
             //addVTCInU(curr->tcnext, v);
             for(elist_t* i = slHead->enext.load(memory_order_seq_cst) ; i->enext.load(memory_order_seq_cst)  != NULL; i = i->enext.load(memory_order_seq_cst) ){
            // cout<<"R-i->val:"<<i->val<<endl;
                     if(i->val < 0 ){ //check for incoming edge  //&& findVSameCC(slHead, &pred, &curr, i->val)
                     //findVSameCC(slHead, &pred, &curr, i->val)
                     bool flag1 = WFContainsV(&curr, (-1)*i->val);
                     if(flag1 == true){
                  //        cout<<"R- Bcurr & i:"<<curr->val<<" "<<i->val<<endl;        
                          remVTCfromU(curr, v);      
                          RDFSBW(curr->enext.load(memory_order_seq_cst) , v);  
                          }
                     }
                   }  
            
        }

     
      bool updateTCAfterRemoveE(vlist_t *pred1, vlist_t *curr1, vlist_t *pred2, vlist_t * curr2){
            tclist_t * itc;
            elist_t * ite1, * ite2;
            vlist_t *n1,*n2;
           elist_t *curre1 = curr1->enext.load(memory_order_seq_cst) , *curre2 = curr2->enext.load(memory_order_seq_cst) ;
           tclist_t *c2 = curr2->tcnext.load(memory_order_seq_cst) ->tcnext.load(memory_order_seq_cst) ;
           // remove all reachable vertixe of curr2 from curr1 and then do the DFSBW
           while(c2->tcnext.load(memory_order_seq_cst)  != NULL) {
             RemoveTC(curr1->tcnext, c2->val);
             c2 = c2->tcnext.load(memory_order_seq_cst) ;
            } 
           for(ite1 = curre1->enext.load(memory_order_seq_cst) ; ite1->enext.load(memory_order_seq_cst)  != NULL; ite1 = ite1 ->enext.load(memory_order_seq_cst) ){
          // cout<<"R-ite1->val:"<<ite1->val<<endl;
              if(ite1->val < 0){
                bool flag1 = WFContainsV(&n1, (-1)*ite1->val);
                if(flag1 == true){
                    remVTCfromU(n1, curr2);  // add all rechable vertices to n1
                    RDFSBW(n1->enext.load(memory_order_seq_cst) , curr2); // call the DFS on the backward vertices 
                  }
               }     
         
         }
       return true;
     }        
    
    void PrintSMTC(){
	cout<<"VHead("<<VHead->val<<endl;
	vlist_t *vtemp = VHead->vnext;
	tclist_t * tctemp;
	elist_t *   etemp;
	while(vtemp){
		cout << vtemp->val << "->";
		etemp = vtemp->enext;
		while(etemp){
			cout << etemp->val << "->";			
			etemp = etemp->enext;
			}
		cout<<"\n  =>";	
		tctemp = vtemp->tcnext;
		while(tctemp){
			cout << tctemp->val << "->";			
			tctemp = tctemp->tcnext;
			}
		vtemp = vtemp->vnext;
		cout<<endl;
	   }
	cout<<")VTail"<<endl;	   
        }               
};

class SMTC{
       // vlist_t *VHead, *VTail;
        Node lt;
        // for initilization of the list
      public:  
        
        bool AddVertexSMTC(int key){
                return lt.AddV(key);
           }  
       
       
           bool RemoveEdgeSMTC(int key1, int key2){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                tclist_t *predn1, *currn1, *predn2, *currn2;
                bool flag1 = lt.WFContainsV(&curr1, key1);
                bool flag2 = lt.WFContainsV(&curr2, key2);
                if(flag1 == false || flag2 == false)
                       return false;
                bool flag3 = lt.WFContainsV(&curr1, key1);
                if(flag3 == false)
                     return false;
                             /*Removes the edge node in the edge list of a given vertex*/        
               bool flag =  lt.RemoveE(curr1->enext.load(memory_order_seq_cst) , key2) && lt.RemoveE(curr2->enext.load(memory_order_seq_cst) , (-1)*key1);  
              if(flag == true){
                 bool flag4 =  lt.RemoveTC(curr1->tcnext.load(memory_order_seq_cst) , key2);
                 if(flag4 == true)  
                  return lt.updateTCAfterRemoveE(pred1, curr1, pred2, curr2);
                 else 
                   return false; 
               }
            return flag;  
        }
        
       bool ContainsVertexSMTC(int key){
                 vlist_t *g1;
                 if(lt.ContainsV(&g1, key)){
                   return true;
                   }
                 else
                    return false;  
        } 
        
        
        bool ContainsEdgeSMTC(int key1, int key2){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                tclist_t *predn1, *currn1, *predn2, *currn2;
                bool flag1 = lt.WFContainsV(&curr1, key1);
                bool flag2 = lt.WFContainsV(&curr2, key2);
                if(flag1 == false || flag2 == false)
                       return false;
                bool flag3 = lt.WFContainsV(&curr1, key1);
                if(flag3 == false)
                     return false;
                 elist_t* pred, * curr;
                 lt.locateE(curr1->enext.load(memory_order_seq_cst) , &pred, &curr, key2);
                 if(curr->val == key2){
                        //pred ->enext = curr->enext;
                        return true;
                }
                else
                        return false;
                        
        }
        
        bool DescendantSMTC(int key1, int key2){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                tclist_t *predn1, *currn1, *predn2, *currn2;
                bool flag1 = lt.ContainsV(&curr1, key1);
                bool flag2 = lt.ContainsV(&curr2, key2);
                if(flag1 == false || flag2 == false)
                       return false;
                bool flag3 = lt.ContainsV(&curr1, key1);
                if(flag3 == false)
                     return false;
                 elist_t* pred, * curr;
                return lt.ContainsTC(curr1->tcnext.load(memory_order_seq_cst), &currn1, key2);
          }
          
           int CountDescendantSMTC(int key){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                tclist_t *predn1, *currn1, *predn2, *currn2;
                bool flag = lt.ContainsV(&curr1, key);
                if(flag == false )
                       return false;
                tclist_t* pred=curr1->tcnext.load(memory_order_seq_cst)->tcnext.load(memory_order_seq_cst);
                int tccount=0;
                while(pred->tcnext!=NULL){
                  tccount ++;
                  pred =pred->tcnext;
                }
                return tccount;
          }     
        
 void  PrintSMTCGraph(){
      lt.PrintSMTC();
 
 }
    
       
   void initSMTC(int n){
          int i,j;
          for(i = 1; i <= n; i++){
            AddVertexSMTC(i);
          }
          
          /*for(i=1; i<=n; i++)
            for(j=i+1; j<=n; j++)
              AddEdgeSMTC(i,j);
        */
        }
    

};


#endif

