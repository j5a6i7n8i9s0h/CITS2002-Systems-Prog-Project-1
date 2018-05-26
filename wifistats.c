/*
   CITS2002 Project 1 2017
   Name(s):             Robin Markwitz (,Jainish Pithadiya)
   Student number(s):   21968594 (, 21962504)
   Date:                Friday 22nd September
*/
// cc -std=c99 -Wall -Werror -pedantic -o rot wifistats.c
#include<stdio.h>
#include<stdlib.h> 
#include<string.h>
#include <limits.h>
#include <stdbool.h> 
#include <ctype.h>

//index for new macadresses to be added. 
int macNote=0;

bool mystrcmp(char s1[],char s2[]){
    int j=strlen(s1);
    for(int i=0;i<j;i++){
        if((i-2)%3==0)continue;
        int a= tolower(s1[i]);
        int b =tolower(s2[i]);
        if(a!=b) return false;
    }
    return true;
}

int isIN(char address[],char* ar[],int maxmacs){
    int isin;
    for(int i=0;i<macNote;i++){
        if(mystrcmp(address,ar[i])){
            return i;
        }
    }
    ar[macNote]= (char *) malloc(17*sizeof(char));
    strcpy(ar[macNote],address);
    isin=macNote;
    macNote++;
    maxmacs--;
    if(maxmacs==0){
        fprintf(stderr,"Surpassed 500 distinct mac's");
        exit(EXIT_FAILURE);
    }
    return isin;
}

void recieve(char filename[], char fileoui[]){
    FILE* file=fopen(filename,"r");
    if(file==NULL) {
        fprintf(stderr,"file couldnt be opened for %s",filename);
     }
    int tr[1000];
    char* ma[500];
    char buff[500];
    int maxmacs=500;
    if(strcmp(fileoui,"FILENA")==0){
        while(fgets(buff,1000,file)!=NULL){
            int noOftabs=0;
            int len= strlen(buff);
            int macpos;
            int k=0;
            for(int i=0;i<len;i++){
                char mac[18];
                char packets[5];
                if(noOftabs<2 && buff[i]!='\t') continue;
                if(buff[i]=='\t') {noOftabs++; continue;}
                if(noOftabs==2){
                    memcpy(mac,&buff[i],17);
                    mac[17]='\0';
                    i=i+17;
                    i--;   
                    if(mystrcmp(mac,"ff-ff-ff-ff-ff-ff")) continue;
                    macpos= isIN(mac,ma,maxmacs);
                    continue;
                }else{
                        while(1){
                            if(buff[i]=='\n'||buff[i]=='\r') break;
                            k++;
                            i++;
                         }
                    memcpy(packets,&buff[i-k],k+1);
                    tr[macpos]+=atoi(packets);
                }
            }
        }
        for(int i=0;i<macNote;i++) {printf("%s : %i\n",ma[i],tr[i]); free(ma[i]);}
    }else{
        FILE* ven= fopen(fileoui,"r");
        char* vennames[500];
        int tr[500];
        char buff[80];
        if(ven==NULL) {
            fprintf(stderr,"file couldnt be opened for %s",fileoui);
            exit(EXIT_FAILURE);
         }
         while(fgets(buff,80,ven)!=NULL){

         }
    }
}
void transmit(char filename[],char oui[]){
    
}

int main(int argc,char* argv[]){
    if(argc==3){
        //pass FILENA 
        if(strcmp(argv[1],"t")==0){

            exit(EXIT_SUCCESS);    
        }else if(strcmp(argv[1],"r")==0){
            recieve(argv[2],"FILENA");
         exit(EXIT_SUCCESS);
        }else{
            fprintf(stderr,"Please provide what you require from output 't' or 'r' \n");
            exit(EXIT_FAILURE);
        }
    }else{
        if(strcmp(argv[1],"t")==0){

        exit(EXIT_SUCCESS);
        }else if(strcmp(argv[1],"r")==0){


        exit(EXIT_SUCCESS);
        }else{
            fprintf(stderr,"Please provide what you require from output 't' or 'r' \n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}



//end 

