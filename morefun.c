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

#define BROADCAST "ff-ff-ff-ff-ff-ff"
#define VENBROAD "ff-ff-ff"


typedef struct  {
    char macaddress[19];
    char vendor[90];
    int packetsize;
} WifiList;

WifiList list[500]; //struct array to hold all addresses, vendors and total packets
int maxmacs;
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
int isDistinct(char address[]){
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
bool findoui(char oui[],char venname[], char *fileoui){
    FILE* ven= fopen(fileoui,"r");
    if(ven==NULL){
        fprintf(stderr, "OUI file could not be opened\n");
        exit(EXIT_FAILURE);
    }
    char line[BUFSIZ];
    while(fgets(line,BUFSIZ,ven)!=NULL){
        char listoui[9];
        strncpy(listoui,line,8);
        listoui[8]='\0';
        if(mystrcmp(listoui,oui)){
            strncpy(venname,line+9,strlen(line)- 9);
            venname[strlen(line)- 8]='\0';
            fclose(ven);
            return true; 
        }
    }
    fclose(ven);
    return false;
}

char *trim(char *str)
{
    char *end;
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0)  // All spaces?
        return str;
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator
    *(end+1) = 0;
    
    
    return str;
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
    char mac[18];
    char packets[5];
    int macpos;
    if(strcmp(fileoui,"FILENA")==0){
        while(fgets(buff,BUFSIZ,file)!=NULL) {
            const char s[2] = "\t";
            char * token;
            token = strtok(buff,s);
            int count = 0;
            bool broad1=false;
            bool broad2= false;
            macpos=-1;
            while(token != NULL) {
                if(count == 1) {
                    char check[18];
                    strcpy(check,trim(token));
                    check[17]='\0';
                    if(mystrcmp(check,BROADCAST))broad1=true;
                }
                if(count == 2) {
                    strcpy(mac,trim(token));
                    mac[17]='\0';
                    if(mystrcmp(mac,BROADCAST)) broad2= true;
                }
                if((broad1==false && broad2==false)&& !mystrcmp("ff-ff-ff-ff-ff-ff",mac)){
                    macpos = isDistinct(mac);
                }else if(broad1==true||broad2==true) break;
                if (count == 3 && macpos!=-1) {
                    strcpy(packets,token);
                    token[4] = '\0';
                    list[macpos].packetsize +=atoi(packets);
                }
                token = strtok(NULL, s);
                count++;
            }
        }
        FILE* f = fopen("result", "w");
        for(int i = 1; i < macNote; i++) {
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
        // array 1 holds mac address (ff:ff:ff) and array 2 holds Vendor names (noone)
        // string to hold each line from file packets.
        char buff2[BUFSIZ];
        // index to note position of mac address and vendor in arrays macVen and Vendors.
        int index;
        while(fgets(buff2,BUFSIZ,file)!=NULL){
            char vendorname[90];
            int noOftabs=0;
            int maxouis=25000;
            int len = strlen(buff2);
            int k=0;
            bool broadcast1=false;
            bool broadcast2=false;
            // macpos serves same purpose as before and holds index. Everything is same here on out till finding value of index
            int macpos;
            for(int i=0;i<len;i++){
                char mac[10];
                char packets[5];
                if(noOftabs<1 && buff2[i]!='\t') continue;
                if(buff2[i]=='\t') {noOftabs++; continue;}
                if(noOftabs==1){
                   char check[9];
                    memcpy(check,&buff2[i],8);
                    check[8]='\0';
                    i+=16;
                    if(mystrcmp(check,VENBROAD))broadcast2=true; 
                }
                if(noOftabs==2){
                memcpy(mac,&buff2[i],8);
                    mac[8]='\0';
                    i=i+16;
                    //if broadcast address encountered.
                    if(mystrcmp(mac,VENBROAD)) broadcast1=true; }
                if(broadcast1==false&& broadcast2==false ){
                    //if no oui exists.
                    if(!findoui(mac,vendorname,fileoui)){strcpy(vendorname,"UNKNOWN-VENDOR"); strcpy(mac,"??:??:??");}
                    macpos=isDistinct(mac);
                    strcpy(list[macpos].vendor,vendorname);
                }else {break;}
                if(noOftabs==3){
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
    char mac[18];
    char packets[5];
    int macpos;
    if(strcmp(fileoui,"FILENA")==0){
        while(fgets(buff,BUFSIZ,file)!=NULL) {
            const char s[2] = "\t";
            char * token;
            token = strtok(buff,s);
            int count = 0;
            bool broad1=false;
            bool broad2= false;
            macpos=-1;
            while(token != NULL) {
                if(count == 1)  {
                    strcpy(mac,trim(token));
                    if(mystrcmp(mac,BROADCAST)) {broad1= true;}
                }
                if(count == 2) {
                    char check[18];
                    strcpy(check,trim(token));
                    if(mystrcmp(check,BROADCAST)) {broad2=true;}
                }
                if(broad1==false && broad2==false){
                    macpos = isDistinct(mac);
                }else if(broad1==true||broad2==true) break;
                if (count == 3 && macpos!=-1) {
                    strcpy(packets,token);
                    token[4] = '\0';
                    list[macpos].packetsize +=atoi(packets);
                }
                token = strtok(NULL, s);
                count++;
            }
        }
        FILE* f = fopen("result", "w");
        for(int i = 1; i < macNote; i++) {
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
    }/*
    else{
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
            int maxouis=25000;
            int len = strlen(buff2);
            int k=0;
            bool broadcast1=false;
            bool broadcast2=false;
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
                    //if broadcast address encountered.
                    if(mystrcmp(mac,VENBROAD)) broadcast1=true;
                }
                if(noOftabs==2){
                    char check[9];
                    memcpy(check,&buff2[i],8);
                    check[8]='\0';
                    i+=16;
                    if(mystrcmp(check,VENBROAD))broadcast2=true;
                }
                if(broadcast1==false&& broadcast2==false ){
                    index=findoui(mac,macVen,line);
                    //if no oui exists.
                    if(index==-1) strcpy(mac,"??:??:??");
                    macpos=isDistinct(mac);
                    if(index!=-1){
                        strcpy(list[macpos].vendor,Vendor[index]);
                    }else{
                        strcpy(list[macpos].vendor,"UNKNOWN-VENDOR") ;
                    }
                }else if(broadcast1==true || broadcast2==true){break;}
                if(noOftabs==3){
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
    */
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

//____________________________________________________________________________________________________________________________________________//
/*
 
 THINGS ADDED SINCE LAST COMMIT
 - it works i think. idk. test it masterbate it flick it idk.
 
 THINGS TO IMPROVE
 1) variable names
 ANSWER: I think they are fine tbh, I don't know if there's any point changing them.
 2) efficieny
 ANSWER: I don't see how we could make it more efficient. You could probably shorten the code
 but again I don't think we should worry about that until we know we're done with everything
 3) other stuff you see fit
 ANSWER: I have made the output look nicer, added comments to explain what happens in each function,
 changed some code layout because Chris may nitpick.
 
 */


/*
 
 THINGS LEFT
 1) C L E A N   U P   T H E   C O D E. Seriously, it's 500 lines and looks like pure shit.
 Sorry :(
 2) Check if it runs on Macs in the CSSE building.
 3) Sorting vendors by alphabets if two vendors have the same packetvalues.
 
 */


/*
 
 HOW TO LOGIN TO THE CSSE MACS USING YOUR COMPUTER
 1) Go into bash. Type $ ssh yourstudentno@uggp.csse.uwa.edu.au
 2) Type yes if prompted, then give your password. NOTE that it looks like it doesn't work, but
 a shell doesn't show you a dot for each character of your password!
 3) Type $ uname -a and you see what system you're using. It should be Linux.
 4) Type $ ssh 130.95.252.xxx. Try xxx = 114 all the way to xxx = 150 to try to get in to any of the computers in the lab.
 You're in!
 
 */


/*
 
 QUESTIONS
 1) for part 2, is distinct mac addresses for the first 3 bytes or the whole mac address?
 ANSWER: I think it is for the whole address. The first three bytes only indicate the vendor -
 a vendor may have many devices, each operating under different MAC addresses.
 
 */

//________________________________________________________________________________________________________________________________________________//



