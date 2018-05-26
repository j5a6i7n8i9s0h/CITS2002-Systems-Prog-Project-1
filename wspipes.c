/*
 CITS2002 Project 1 2017
 Name(s):             Robin Markwitz (,Jainish Pithadiya)
 Student number(s):   21968594 (, 21962504)
 Date:                Friday 22nd September
 */
// cc -std=c99 -Wall -Werror -pedantic -o rot ws2.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>


typedef struct  {
    char macaddress[19];
    char vendor[90];
    int packetsize;
} WifiList;

WifiList list[500]; //struct array to hold all addresses, vendors and total packets
int macNote=0; //index for new macadresses to be added. Used accross all methods and holds same definition.
char* Arguments[]={"/usr/bin/sort","-t", "\t", "-k2,2rn", "-k1,1", "result",(char*)NULL};
char* ArgumentsOui[]={"/usr/bin/sort","-t", "\t", "-k3,3rn", "-k2,2", "result",(char *)NULL};

/*
 Different implementation of strcmp in order to ignore upper or lower case.
 */
bool mystrcmp(char s1[],char s2[]) {
    int j=strlen(s1);
    for(int i=0;i<j;i++){
        if((i-2)%3==0)continue;
        int a= tolower(s1[i]);
        int b =tolower(s2[i]);
        if(a!=b) return false;
    }
    return true;
}
/*
 When a MAC address is found, this function compares it to previously found addresses
 that are in the struct and adds it IF it is distinct.
 */
int isDistinct(char address[],int maxmacs){
    int isin;
    for(int i=0;i<macNote;i++){
        if(mystrcmp(address,list[i].macaddress)){
            return i;
        }
    }
    strcpy(list[macNote].macaddress,address);
    isin=macNote;
    macNote++;
    maxmacs--;
    if(maxmacs==0){
        fprintf(stderr,"Surpassed 500 distinct mac's\n");
        exit(EXIT_FAILURE);
    }
    return isin;
}
/*
 This function finds the index that links the address to the vendor name where 2 arrays
 have the same indexing for particular MAC addresses and vendors.
 For example: mac[2]= ff:ff:ff , vendor[2]= Noone.
 mac address ff:ff:ff has vendor noone.
 findoui gets this index for a given address.
 Hence findoui("ff:ff:ff",mac,line) will return 2.
 NOTE: This function returns -1 if no vendor is found.
 */
int findoui(char threemac[],char *mac[],int line) {
    for(int i=0;i<line;i++)if(mystrcmp(threemac,mac[i])) return i;
    return -1;
}
/*
 This function, given two filenames, will fill up the struct array list[] and then print it out.
 It does this by using arrays to temporarily store data until it is converted into the appropriate
 output form, at which point it is added to the struct array.
 If the function receives no OUI file as an argument, it will simply print out the MAC addresses and total number of packets.
 However if it does receive an OUI file, it will print out the addresses, the vendors and then the packets.
 This function prints out the statistics for WiFi receivers.
 */
void receive(char filename[], char fileoui[]){
    FILE* file=fopen(filename,"r");
    if(file==NULL) {
        fprintf(stderr, "File could not be opened\n");
        exit(EXIT_FAILURE);
    }
    char buff[BUFSIZ];
    int maxmacs=500;
    if(strcmp(fileoui,"FILENA")==0){
        while(fgets(buff,BUFSIZ,file)!=NULL){
            int noOftabs=0;
            int len = strlen(buff);
            int macpos;
            int k=0;
            for(int i=0;i<len;i++){
                char mac[18];
                char packets[5];
                if(noOftabs<2 && buff[i]!='\t') continue;
                if(buff[i]=='\t') {noOftabs++; continue;}
                if(noOftabs==1){
                    char check[18];
                    memcpy(check,&buff[i],17);
                    check[17]='\0';
                    if(mystrcmp(check,"ff-ff-ff-ff-ff-ff")){
                        macpos=-1;
                    }
                }
                if(noOftabs==2){
                    memcpy(mac,&buff[i],17);
                    mac[17]='\0';
                    i+=16;
                    if(mystrcmp(mac,"ff-ff-ff-ff-ff-ff")){noOftabs=4; continue;}
                    macpos= isDistinct(mac,maxmacs);
                }
                else{
                    if(macpos!=-1){
                    while(1){
                        if(buff[i]=='\n'||buff[i]=='\r') break;
                        k++;
                        i++;
                    }
                    memcpy(packets,&buff[i-k],k+1);
                    list[macpos].packetsize +=atoi(packets);
                }
                }
            }
        }
	
        FILE* f = fopen("result", "w");
        for(int i = 0; i < macNote; i++) {
            fprintf(f, "%s\t%d\n" ,list[i].macaddress ,list[i].packetsize);
        }
        if(f != NULL) fclose(f);
        pid_t pid;                 // process ID
        int status;
        switch (pid = fork()){
            case -1 :
                perror("fork()");     // process creation failed
                exit(1);
                break;
                
            case 0:                 //new child process
                execv("/usr/bin/sort",Arguments);
                printf("dont work\n");
                _exit(1);
                break;
            default:
                wait(&status);
                remove("result");
                break;
                
        }
        if(file != NULL) fclose(file);
    }
    else{
        // perform processing on fileoui as provided by user.
        FILE* ven= fopen(fileoui,"r");
        if(ven==NULL){
            fprintf(stderr, "OUI file %s could not be opened\n",fileoui);
            exit(EXIT_FAILURE);
        }
        // array 1 holds mac address (ff:ff:ff) and array 2 holds Vendor names (noone)
        char* macVen[25000];
        char* Vendor[25000];
        // notes amount of OUI'S
        // line holds notes the number of vendors by fileoui.
        int line=0;
        while(fgets(buff,BUFSIZ,ven)!=NULL){
            // holds mac address ff:ff:ff for each line.
            char hex[10];
            int venIndex= strlen(buff)- 9;
            if(venIndex>90){
                fprintf(stderr,"Vendor name is too long at line %i for file %s. Exceeded by 90-%i characters",line+1,fileoui,venIndex);
                exit(EXIT_FAILURE);
            }
            // holds vendor name for mac address string hex, i.e in the example that would be Noone.
            char venname[venIndex+1];
            memcpy(venname,&buff[9],venIndex-1);
            venname[venIndex-1]='\0';
            memcpy(hex,&buff[0],8);
            hex[9]='\0';
            //dynamic memory allocation
            macVen[line]=(char*) malloc(9*sizeof(char));
            Vendor[line]=(char*) malloc(venIndex*sizeof(char));
            strcpy(macVen[line],hex);
            strcpy(Vendor[line++],venname);
        }
        // string to hold each line from file packets.
        char buff2[BUFSIZ];
        // index to note position of mac address and vendor in arrays macVen and Vendors.
        int index;
        while(fgets(buff2,BUFSIZ,file)!=NULL){
            int noOftabs=0;
            int  maxouis=25000;
            int len = strlen(buff2);
            int k=0;
            // macpos serves same purpose as before and holds index. Everything is same here on out till finding value of index
            int macpos;
            for(int i=0;i<len;i++){
                char mac[10];
                char packets[5];
                if(noOftabs<2 && buff2[i]!='\t') continue;
                if(buff2[i]=='\t') {noOftabs++; continue;}
                if(noOftabs==2){
                    memcpy(mac,&buff2[i],8);
                    mac[8]='\0';
                    i=i+16;
                    if(mystrcmp(mac,"ff-ff-ff")) continue;
                    //if vendor name is provided in oui file retrieve index for macVen and Vendor array. If not found return -1.
                    index=findoui(mac,macVen,line);
                    //if no oui exists.
                    if(index==-1) strcpy(mac,"??:??:??");
                    macpos=isDistinct(mac,maxouis);
                    if(index!=-1){
                        strcpy(list[macpos].vendor,Vendor[index]);
                    }else{
                        strcpy(list[macpos].vendor,"UNKNOWN-VENDOR") ;
                    }
                    continue;
                }
                else if(noOftabs==3){
                    while(1){
                        if(buff2[i]=='\n'||buff2[i]=='\r') break;
                        k++;
                        i++;
                    }
                    memcpy(packets,&buff2[i-k],k+1);
                    packets[k+1]='\0';
                    list[macpos].packetsize +=atoi(packets);
                }
            }
        }
        if(file!=NULL) fclose(file);
        FILE* f = fopen("result", "w");
        for(int i=0;i<macNote;i++){
            fprintf(f,"%s\t%s\t%i\t\n",list[i].macaddress,list[i].vendor,list[i].packetsize);
        }
        if(f!=NULL) fclose(f);
        for(int i=0;i<line;i++){
            // free malloc'd memory once done.
            free(macVen[i]);
            free(Vendor[i]);
        }
        pid_t pid;                 // process ID
        int status;
        switch (pid = fork()){
            case -1 :
                perror("fork()");     // process creation failed
                exit(1);
                break;
                
            case 0:                 //new child process
                execv("/usr/bin/sort",ArgumentsOui);
                printf("dont work\n");
                _exit(1);
                break;
            default:
                wait(&status);
                remove("result");
                break;
                
        }
        if(ven != NULL) fclose(ven);
        exit(EXIT_SUCCESS);
    }
}

/*
 This function, given two filenames, will fill up the struct array list[] and then print it out.
 It does this by using arrays to temporarily store data until it is converted into the appropriate
 output form, at which point it is added to the struct array.
 If the function receives no OUI file as an argument, it will simply print out the MAC addresses and total number of packets.
 However if it does receive an OUI file, it will print out the addresses, the vendors and then the packets.
 This function prints out the statistics for WiFi transmitters.
 */
void transmit(char filename[], char fileoui[]){
    FILE* file=fopen(filename,"r");
    if(file==NULL) {
        fprintf(stderr, "File could not be opened\n");
        exit(EXIT_FAILURE);
    }
    char buff[BUFSIZ];
    int maxmacs=500;
    if(strcmp(fileoui,"FILENA")==0){
        while(fgets(buff,BUFSIZ,file)!=NULL){
            int noOftabs=0;
            int len = strlen(buff);
            int macpos;
            int k=0;
            for(int i=0;i<len;i++){
                char mac[18];
                char packets[5];
                if(noOftabs<1 && buff[i]!='\t') continue;
                if(buff[i]=='\t') {noOftabs++; continue;}
                if(noOftabs==1){
                    memcpy(mac,&buff[i],17);
                    mac[17]='\0';
                    i+=16;
                    if(mystrcmp(mac,"ff-ff-ff-ff-ff-ff")){noOftabs=4; continue;}
                    macpos= isDistinct(mac,maxmacs);
                }
                if(noOftabs==2){
                    char check[18];
                    memcpy(check,&buff[i],17);
                    check[17]='\0';
                    i+=16;
                    if(mystrcmp(check,"ff-ff-ff-ff-ff-ff")){
                        macpos=-1;
                    }
                }
                else if(noOftabs==3){
                    if(macpos!=-1){
                    while(1){
                        if(buff[i]=='\n'||buff[i]=='\r') break;
                        k++;
                        i++;
                    }
                    memcpy(packets,&buff[i-k],k+1);
                    packets[k+1]='\0';
                    list[macpos].packetsize +=atoi(packets);
                }
                }
            }
        }
	
        FILE* f = fopen("result", "w");
        for(int i = 0; i < macNote; i++) {
            fprintf(f, "%s\t%d\n" ,list[i].macaddress ,list[i].packetsize);
        }
        if(f != NULL) fclose(f);
        pid_t pid;                 // process ID
        int status;
        switch (pid = fork()){
            case -1 :
                perror("fork()");     // process creation failed
                exit(1);
                break;
                
            case 0:                 //new child process
                execv("/usr/bin/sort",Arguments);
                printf("dont work\n");
                _exit(1);
                break;
            default:
                wait(&status);
                remove("result");
                break;
                
        }
        //printList(list);
        if(file != NULL) fclose(file);
        
    }
    else{
        // perform processing on fileoui as provided by user.
        FILE* ven= fopen(fileoui,"r");
        if(ven==NULL){
            fprintf(stderr, "OUI file could not be opened\n");
            exit(EXIT_FAILURE);
        }
        // array 1 holds mac address (ff:ff:ff) and array 2 holds Vendor names (noone)
        char* macVen[25000];
        char* Vendor[25000];
        // notes amount of OUI'S
        // line holds notes the number of vendors by fileoui.
        int line=0;
        while(fgets(buff,BUFSIZ,ven)!=NULL){
            // holds mac address ff:ff:ff for each line.
            char hex[10];
            int venIndex= strlen(buff)- 9;
            if(venIndex>90){
                fprintf(stderr,"Vendor name is too long at line %i for file %s. Exceeded by 90-%i characters",line+1,fileoui,venIndex);
                exit(EXIT_FAILURE);
            }
            // holds vendor name for mac address string hex, i.e in the example that would be Noone.
            char venname[venIndex+1];
            memcpy(venname,&buff[9],venIndex-1);
            venname[venIndex-1]='\0';
            memcpy(hex,&buff[0],8);
            hex[9]='\0';
            //dynamic memory allocation
            macVen[line]=(char*) malloc(9*sizeof(char));
            Vendor[line]=(char*) malloc(venIndex*sizeof(char));
            strcpy(macVen[line],hex);
            strcpy(Vendor[line++],venname);
        }
        // string to hold each line from file packets.
        char buff2[BUFSIZ];
        // index to note position of mac address and vendor in arrays macVen and Vendors.
        int index;
        while(fgets(buff2,BUFSIZ,file)!=NULL){
            int noOftabs=0;
             int  maxouis=25000;
            int len = strlen(buff2);
            int k=0;
            // macpos serves same purpose as before and holds index. Everything is same here on out till finding value of index
            int macpos;
            for(int i=0;i<len;i++){
                char mac[10];
                char packets[5];
                if(noOftabs<1 && buff2[i]!='\t') continue;
                if(buff2[i]=='\t') {noOftabs++; continue;}
                if(noOftabs==1){
                    memcpy(mac,&buff2[i],8);
                    mac[8]='\0';
                    i=i+16;
                    if(mystrcmp(mac,"ff-ff-ff")) break;
                    //if vendor name is provided in oui file retrieve index for macVen and Vendor array. If not found return -1.
                    index=findoui(mac,macVen,line);
                    //if no oui exists.
                    if(index==-1) strcpy(mac,"??:??:??");
                    macpos=isDistinct(mac,maxouis);
                    if(index!=-1){
                        strcpy(list[macpos].vendor,Vendor[index]);
                    }else{
                        strcpy(list[macpos].vendor,"UNKNOWN-VENDOR") ;
                    }
                    continue;
                }
                if(noOftabs==2){
                    char check[9];
                    memcpy(check,&buff2[i],8);
                    check[8]='\0';
                    i+=16;
                    if(mystrcmp(check,"ff-ff-ff")){
                        break;
                    }
                }
                else if(noOftabs==3){
                    while(1){
                        if(buff2[i]=='\n'||buff2[i]=='\r') break;
                        k++;
                        i++;
                    }
                    memcpy(packets,&buff2[i-k],k+1);
                    packets[k+1]='\0';
                    list[macpos].packetsize +=atoi(packets);
                }
            }
        }
        FILE* f = fopen("result", "w");
        for(int i=0;i<macNote;i++){
            fprintf(f,"%s\t%s\t%i\t\n",list[i].macaddress,list[i].vendor,list[i].packetsize);
        }
        for(int i=0;i<line;i++){
            // free malloc'd memory once done.
            free(macVen[i]);
            free(Vendor[i]);
        }
        if(f != NULL) fclose(f);
        pid_t pid;                 // process ID
        int status;
        switch (pid = fork()){
            case -1 :
                perror("fork()");     // process creation failed
                exit(1);
                break;
                
            case 0:                 //new child process
                execv("/usr/bin/sort",ArgumentsOui);
                printf("dont work\n");
                _exit(1);
                break;
            default:
                wait(&status);
                remove("result");
                break;
                
        }
        if(ven != NULL) fclose(ven);
    }
}

/*
 Handles the input given to the C file from the command line.
 Distinguishes between the many types of input that the program must support.
 */
int main(int argc,char* argv[]){
    if(argc==3){

        if(strcmp(argv[1],"t")==0){
            transmit(argv[2],"FILENA");
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[1],"r")==0){
            receive(argv[2],"FILENA");
            exit(EXIT_SUCCESS);
        }
        else {
            fprintf(stderr,"Please provide what you require from output 't' or 'r' \n");
            exit(EXIT_FAILURE);
        }
    }
    else if(argc==4){
        if(strcmp(argv[1],"t")==0){
            transmit(argv[2],argv[3]);
            exit(EXIT_SUCCESS);
        }
        else if(strcmp(argv[1],"r")==0){
            receive(argv[2],argv[3]);
            exit(EXIT_SUCCESS);
        }
        else {
            fprintf(stderr,"Please provide what you require from output 't' or 'r' \n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr,"Provide input in ./program what packets ouifile(optional)\n" );
    }
    return 0;
}

//end



