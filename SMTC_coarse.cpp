/*
* TC_Graph_Coarse.cpp:
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
	struct Enode *enext; // pointer to the next adjancy list
}elist_t;

typedef struct TCnode{
	int val; // data
	struct TCnode *tcnext; // pointer to the next adjancy list
}tclist_t;

typedef struct Vnode{
	int val; // data
	struct Enode *enext;// pointer to the elist
	struct TCnode *tcnext; // pointer to the tclist
	struct Vnode *vnext; // pointer to the vlist
}vlist_t;

class Node{
           vlist_t *VHead, *VTail;
 public:
        Node(){
                VHead = (vlist_t*) malloc(sizeof(vlist_t));
                VHead ->val = INT_MIN;
                VHead ->enext = NULL;
                VHead ->tcnext = NULL;
                VHead ->vnext = NULL;
                VTail = (vlist_t*) malloc(sizeof(vlist_t));
                VTail ->val = INT_MAX;
                VTail ->enext = NULL;
                VTail ->tcnext = NULL;
                VTail ->vnext = NULL;
                VHead->vnext=VTail;
           }
                
        
         elist_t* createE(int key){
                elist_t * temp = (elist_t*) malloc(sizeof(elist_t));
                temp ->val = key;
                temp ->enext = NULL;
                return temp;
        }  
         tclist_t* createTC(int key){
                tclist_t * temp = (tclist_t*) malloc(sizeof(tclist_t));
                temp ->val = key;
                temp ->tcnext = NULL;
                return temp;
        }  
        vlist_t* createV(int key){
                elist_t * EHead = createE(INT_MIN);
                elist_t * ETail = createE(INT_MAX);
                EHead->enext = ETail;
                tclist_t * TCHead = createTC(INT_MIN);
                tclist_t * TCTail = createTC(INT_MAX);
                tclist_t * newtc = createTC(key);
                newtc->tcnext = TCTail;
                TCHead->tcnext = newtc;
                vlist_t* newv = (vlist_t*) malloc(sizeof(vlist_t));
                newv->val = key;
                newv->enext = EHead;
                newv->vnext = NULL;
                newv->tcnext = TCHead;
                return newv;
        }
        void locateE(elist_t* Ehead, elist_t ** n1, elist_t ** n2, int key){
                elist_t* pred = Ehead; 
                elist_t* curr = pred->enext; 
                while( curr->enext != NULL && curr->val < key){ //
                        pred = curr;
                        curr = curr->enext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                 }
        void locateV(vlist_t ** n1, vlist_t ** n2, int key){
                 
                vlist_t* pred = VHead; 
                vlist_t* curr = pred->vnext; 
                while(curr->vnext != NULL && curr->val < key){
                        pred = curr;
                        curr = curr->vnext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                }
        void locateTC(tclist_t* TChead, tclist_t ** n1, tclist_t ** n2, int key){
                 
                tclist_t* pred = TChead; 
                tclist_t* curr = pred->tcnext; 
                while(curr->tcnext != NULL && curr->val < key){
                        pred = curr;
                        curr = curr->tcnext;
                       }
                (*n1) = pred;
                (*n2) = curr;
                }  
         bool ContainsE(elist_t *Ehead, elist_t ** n, int key){
                elist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return sutcess, true
                }
                else
                        return false; // key not present
            }             
         bool ContainsV(vlist_t ** n, int key){
                vlist_t* pred, *curr;
                locateV(&pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return sutcess, true
                }
                else
                        return false; // key not present
        }
        bool ContainsTC(tclist_t *TChead, tclist_t ** n, int key){
                tclist_t* pred, *curr;
                locateTC(TChead, &pred, &curr, key);
                if(curr->val == key){
                        (*n) = curr; 
                        return true;       // found it, return sutcess, true
                }
                else
                        return false; // key not present
        }
        bool AddE(elist_t *Ehead, int key){
                elist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                elist_t *newe = createE(key);
                if(curr->val != key){
                        newe ->enext = curr;   
                        pred ->enext = newe;
                        return true;
                }
                else
                 return false;
          }  
         bool AddTC(tclist_t *TChead, int key){
                tclist_t* pred, *curr;
                locateTC(TChead, &pred, &curr, key);
                tclist_t *newe = createTC(key);
                if(curr->val != key){
                        newe ->tcnext = curr;   
                        pred ->tcnext = newe;
                        return true;
                }
                else
                 return false;
          }  
           bool AddV(int key){
                vlist_t* pred, *curr;
                locateV(&pred, &curr, key);
                vlist_t *newv = createV(key);
                if(curr->val != key){
                        newv ->vnext = curr;   
                        pred ->vnext = newv;
                        return true;
                }
                else
                 return false;
               }
         bool RemoveE(elist_t *Ehead, int key){
                elist_t* pred, *curr;
                locateE(Ehead, &pred, &curr, key);
                if(curr->val == key){
                       pred ->enext = curr->enext;
                        curr->enext = NULL;
                        free(curr);
                        return true;
                }
                else
                 return false;
          }  
          bool RemoveTC(tclist_t *TChead, int key){
                tclist_t* pred, *curr;
                locateTC(TChead, &pred, &curr, key);
                if(curr->val == key){
                       pred ->tcnext = curr->tcnext;
                        curr->tcnext = NULL;
                        free(curr);
                        return true;
                }
                else
                 return false;
          }  
          
         bool RemoveV(int key, vlist_t **n1, vlist_t **n2){
                vlist_t* pred, *curr;
                locateV(&pred, &curr, key);
                (*n1) = pred; (*n2) = curr;
                if(curr->val == key){
                        
                        pred ->vnext = curr->vnext;
                        curr->vnext=NULL;
                        free(curr);
                        return true;
                }
                else
                 return false;
               }
              
          
         /*DFS of the forward traversal*/
        void DFSFW(elist_t * slHead){
             vlist_t * curr, *pred;//used for iteration of each vertex in findVSameCC
             
             //cout<<"it_edge->val:"<<it_edge->val<<endl;
             //cin>>z;
             for(elist_t* i = slHead->enext->enext; i->enext != NULL; i = i->enext){
             //cout<<"i->val:"<<i->val<<endl;
                     if(i->val > 0 ){ //check for out-going edge   //&& findVSameCC(slHead, &pred, &curr, i->val)
                     //findVSameCC(slHead, &pred, &curr, i->val)
                     bool flag = ContainsV(&curr, i->val);
                     if(flag == true){
                          //cout<<" Fcurr & i:"<<curr->val<<" "<<i->val<<endl;
                          DFSFW(curr->enext);  
                         } 
                     }
                   }  
            
        }
            // add each reachable vertices in vHead to tcHead 
           void  addVTCInU(vlist_t *u, vlist_t *v){
               //tclist_t *pred1 = tcHead, *curr1 = pred->tcnext;
               tclist_t *tempu = u->tcnext, *tempv = v->tcnext->tcnext;
              // cout<<"A-tempu & tempv:"<<tempu->val<<" "<<tempv->val<<endl;
               while(tempv->tcnext != NULL){
                 AddTC(tempu, tempv->val);
                 tempv = tempv->tcnext;
               }
           }   
        /*DFS of the backward traversal*/      
         void DFSBW(elist_t * slHead, vlist_t *v){
             vlist_t * curr, *pred;//used for iteration of each vertex in findVSameCC
             
            // cout<<"A-slHead->val & v->val:"<<slHead->val<<" "<<v->val<<endl;
             //cin>>z;
             //addVTCInU(curr->tcnext, v);
             for(elist_t* i = slHead->enext; i->enext != NULL; i = i->enext){
            // cout<<"A-i->val:"<<i->val<<endl;
                     if(i->val < 0 ){ //check for incoming edge  //&& findVSameCC(slHead, &pred, &curr, i->val)
                     //findVSameCC(slHead, &pred, &curr, i->val)
                     bool flag1 = ContainsV(&curr, (-1)*i->val);
                     if(flag1 == true){
                         // cout<<"A-Bcurr & i:"<<curr->val<<" "<<i->val<<endl;        
                          addVTCInU(curr, v);      
                          DFSBW(curr->enext, v);  
                          }
                     }
                   }  
            
        }
       bool updateTCAfterAddE(vlist_t *pred1, vlist_t *curr1, vlist_t *pred2, vlist_t * curr2){
            tclist_t * itc;
            elist_t * ite1, * ite2;
            vlist_t *n1,*n2;
           elist_t *curre1 = curr1->enext, *curre2 = curr2->enext;
           tclist_t *c2 = curr2->tcnext->tcnext;
           // add all reachable vertixe of curr2 to curr1 and then do the DFSBW
           while(c2->tcnext != NULL) {
             AddTC(curr1->tcnext, c2->val);
             c2 = c2->tcnext;
            } 
           
           for(ite1 = curre1->enext; ite1->enext != NULL; ite1 = ite1 ->enext){
           //cout<<"A-ite1->val:"<<ite1->val<<endl;
              if(ite1->val < 0){
                bool flag1 = ContainsV(&n1, (-1)*ite1->val);
                if(flag1 == true){
                    addVTCInU(n1, curr2);  // add all rechable vertices to n1
                    DFSBW(n1->enext, curr2); // call the DFS on the backward vertices 
                  }
               }     
         
         }
       return true;
     }        
     
       // add each reachable vertices in vHead to tcHead 
           void  remVTCfromU(vlist_t *u, vlist_t *v){
               //tclist_t *pred1 = tcHead, *curr1 = pred->tcnext;
               tclist_t *tempu = u->tcnext, *tempv = v->tcnext->tcnext;
               //cout<<"R-tempu & tempv:"<<tempu->val<<" "<<tempv->val<<endl;
               while(tempv->tcnext != NULL){
                 RemoveTC(tempu, tempv->val);
                 tempv = tempv->tcnext;
               }
           }   
        /*DFS of the backward traversal*/      
         void RDFSBW(elist_t * slHead, vlist_t *v){
             vlist_t * curr, *pred;//used for iteration of each vertex in findVSameCC
             
             //cout<<"R-slHead->val & v->val:"<<slHead->val<<" "<<v->val<<endl;
             //cin>>z;
             //addVTCInU(curr->tcnext, v);
             for(elist_t* i = slHead->enext; i->enext != NULL; i = i->enext){
            // cout<<"R-i->val:"<<i->val<<endl;
                     if(i->val < 0 ){ //check for incoming edge  //&& findVSameCC(slHead, &pred, &curr, i->val)
                     //findVSameCC(slHead, &pred, &curr, i->val)
                     bool flag1 = ContainsV(&curr, (-1)*i->val);
                     if(flag1 == true){
                  //        cout<<"R- Bcurr & i:"<<curr->val<<" "<<i->val<<endl;        
                          remVTCfromU(curr, v);      
                          RDFSBW(curr->enext, v);  
                          }
                     }
                   }  
            
        }

     
      bool updateTCAfterRemoveE(vlist_t *pred1, vlist_t *curr1, vlist_t *pred2, vlist_t * curr2){
            tclist_t * itc;
            elist_t * ite1, * ite2;
            vlist_t *n1,*n2;
           elist_t *curre1 = curr1->enext, *curre2 = curr2->enext;
           tclist_t *c2 = curr2->tcnext->tcnext;
           // remove all reachable vertixe of curr2 from curr1 and then do the DFSBW
           while(c2->tcnext != NULL) {
             RemoveTC(curr1->tcnext, c2->val);
             c2 = c2->tcnext;
            } 
           for(ite1 = curre1->enext; ite1->enext != NULL; ite1 = ite1 ->enext){
          // cout<<"R-ite1->val:"<<ite1->val<<endl;
              if(ite1->val < 0){
                bool flag1 = ContainsV(&n1, (-1)*ite1->val);
                if(flag1 == true){
                    remVTCfromU(n1, curr2);  // add all rechable vertices to n1
                    RDFSBW(n1->enext, curr2); // call the DFS on the backward vertices 
                  }
               }     
         
         }
       return true;
     }        
     
      bool updateTCAfterRemoveV(vlist_t *curr1, vlist_t *curr2){
            tclist_t * itc;
            elist_t * ite1, * ite2;
            vlist_t *n1,*n2;
           elist_t *curre1 = curr1->enext, *curre2 = curr2->enext;
           tclist_t *c2 = curr2->tcnext->tcnext;
           // remove all reachable vertixe of curr2 from curr1 and then do the DFSBW
           while(c2->tcnext != NULL) {
             RemoveTC(curr1->tcnext, c2->val);
             c2 = c2->tcnext;
            } 
           for(ite1 = curre1->enext; ite1->enext != NULL; ite1 = ite1 ->enext){
           //cout<<"RV-ite1->val:"<<ite1->val<<endl;
              if(ite1->val < 0){
                bool flag1 = ContainsV(&n1, (-1)*ite1->val);
                if(flag1 == true){
                    remVTCfromU(n1, curr2);  // add all rechable vertices to n1
                    RDFSBW(n1->enext, curr2); // call the DFS on the backward vertices 
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
         bool RemoveVertexSMTC(int key){
                vlist_t* pred, *curr;
                //return lt.RemoveV(key);
                if(lt.RemoveV(key, &pred, &curr)){
                  bool flag4 =  lt.RemoveTC(pred->tcnext, key);
                 if(flag4 == true)  
                  return lt.updateTCAfterRemoveV(pred, curr);
                 else 
                   return false; 
                }
                return false;
              }                          
        
        bool AddEdgeSMTC(int key1, int key2){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                bool flag1 = lt.ContainsV(&curr1, key1);
                bool flag2 = lt.ContainsV(&curr2, key2);
                if(flag1 == false || flag2 == false)
                       return false;
                bool flag3 = lt.ContainsV(&curr1, key1);
                if(flag3 == false)
                     return false;
                bool flag =  lt.AddE(curr1->enext, key2) && lt.AddE(curr2->enext, (-1)*key1);  
                if(flag == true){
                 bool flag4 =  lt.AddTC(curr1->tcnext, key2);
                 if(flag4 == true)  
                  return lt.updateTCAfterAddE(pred1, curr1, pred2, curr2);
                 else 
                   return false; 
               }
            return flag;  
        }
           bool RemoveEdgeSMTC(int key1, int key2){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                tclist_t *predn1, *currn1, *predn2, *currn2;
                bool flag1 = lt.ContainsV(&curr1, key1);
                bool flag2 = lt.ContainsV(&curr2, key2);
                if(flag1 == false || flag2 == false)
                       return false;
                bool flag3 = lt.ContainsV(&curr1, key1);
                if(flag3 == false)
                     return false;
                             /*Removes the edge node in the edge list of a given vertex*/        
               bool flag =  lt.RemoveE(curr1->enext, key2) && lt.RemoveE(curr2->enext, (-1)*key1);  
              if(flag == true){
                 bool flag4 =  lt.RemoveTC(curr1->tcnext, key2);
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
                bool flag1 = lt.ContainsV(&curr1, key1);
                bool flag2 = lt.ContainsV(&curr2, key2);
                if(flag1 == false || flag2 == false)
                       return false;
                bool flag3 = lt.ContainsV(&curr1, key1);
                if(flag3 == false)
                     return false;
                 elist_t* pred, * curr;
                 lt.locateE(curr1->enext, &pred, &curr, key2);
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
                return lt.ContainsTC(curr1->tcnext, &currn1, key2);
          }
          
           int CountDescendantSMTC(int key){
                vlist_t* pred1, *curr1, *pred2, *curr2;
                tclist_t *predn1, *currn1, *predn2, *currn2;
                bool flag = lt.ContainsV(&curr1, key);
                if(flag == false )
                       return false;
                tclist_t* pred=curr1->tcnext->tcnext, * curr;
                int tccount=0;
                while(pred->tcnext!=NULL){
                  tccount ++;
                  pred =pred->tcnext;
                }
                return lt.ContainsTC(curr1->tcnext, &currn1, key);
          }     
        
 void  PrintSMTCGraph(){
      lt.PrintSMTC();
 
 }
    
       
   void initSMTC(int n){
          int i,j;
          for(i = 1; i <= n; i++){
            AddVertexSMTC(i);
          }
       /*   
          for(i=1; i<=n; i++)
            for(j=i+1; j<=n; j++)
              AddEdgeSMTC(i,j);
       */
        }
    

};


#endif

