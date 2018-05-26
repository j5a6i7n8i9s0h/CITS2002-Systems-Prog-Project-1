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
#include <unistd.h>

//Global variables to denote broadcast MAC addresses and the respective OUI.
#define BROADCAST "ff-ff-ff-ff-ff-ff"
#define VENBROAD "ff-ff-ff"

/*
 This is a struct to store the following
 */
typedef struct  {
    char macaddress[19]; //A distinct MAC address
    char vendor[90]; //Stores an OUI
    int packetsize; //Stores the packets that each distinct MAC address holds
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

// function strtok returns unnecessary white space which affects output of the macaddress. Function gets rid of this white space. 
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
void analyze(char input[], char filename[], char fileoui[]){
    FILE* file=fopen(filename,"r");
    if(file==NULL) {
        fprintf(stderr, "File could not be opened\n");
        exit(EXIT_FAILURE);
    }
    char buff[BUFSIZ];
    char mac[18];// stores macaddress/oui 
    char packets[5];//stores packet value
    int macpos;//Gives index of distinct mac address in struct array. 
  	//if no OUI file is provided
    if(strcmp(fileoui,"FILENA")==0){
        while(fgets(buff,BUFSIZ,file)!=NULL) {
          	// use tabs as tokens to gather required strings using function strtok
            const char s[2] = "\t";
            char * token;
            token = strtok(buff,s);
            int count = 0;// notes amount of tabs which has been read for a specific line. 
          	//Check if the BROADCAST address is in coloumns for specific. If found, noted as true. 
            bool broad1=false; // notes if broadcast address is in Transmit colomn in packets file
            bool broad2= false;// notes if broadcast address is in Recieve colomn in packets file
          	//if macpos is -1, its macaddress is irrelevant.
            macpos=-1;
          	//if what=Recieve
            if(strcmp(input,"r") == 0){
                while(token != NULL) {
                  	// Everything up to the first tab 
                    if(count == 1) {
                        char check[18];// Checks if broadcast address is in transmit coloumn to determine if we need to ignore current line or not 
                        strcpy(check,trim(token));
                        check[17]='\0';
                        if(mystrcmp(check,BROADCAST))broad1=true; //if broadcast address is found in transmit coloumn
                    }
                  	// Everything up to the second tab
                    if(count == 2) {
                        strcpy(mac,trim(token)); 
                        mac[17]='\0';
                        if(mystrcmp(mac,BROADCAST)) broad2= true; //if broadcast address is found in Recieve coloumn
                    }
                  	// if no broadcast signal is found in the line 
                    if((broad1==false && broad2==false)&& !mystrcmp(BROADCAST,mac)){
                        macpos = isDistinct(mac);
                      	//gets positiion of mac address in struct array
                    }else {break;} // if broadcast exists in line, ignore line 
                  	// Sums up all the packets for distinct macaddresse. 
                    if (count == 3 && macpos!=-1) {
                        strcpy(packets,token);
                        token[4] = '\0';
                        list[macpos].packetsize +=atoi(packets);
                    }
                    token = strtok(NULL, s);
                    count++; //proceed to next tab
                }
            }//if what=Transmit 
            else if(strcmp(input,"t") == 0) {
                while(token != NULL) {
                  	// checks if broadcast is in Transmit colomn
                    if(count == 1)  {
                        strcpy(mac,trim(token));
                        if(mystrcmp(mac,BROADCAST)) {broad1= true;}
                    }
                    if(count == 2) {
                        char check[18]; // Checks if broadcast address is in recieve coloumn to determine if we need to ignore current line or not 
                        strcpy(check,trim(token));
                        if(mystrcmp(check,BROADCAST)) {broad2=true;}//if broadcast address is found in recieve coloumn
                    }
                    if(broad1==false && broad2==false){
                        macpos = isDistinct(mac);
                    }
                  	else {break;}
                   // Sums up all the packets for distinct macaddresse.
                    if (count == 3 && macpos!=-1) {
                        strcpy(packets,token);
                        token[4] = '\0';
                        list[macpos].packetsize +=atoi(packets);
                    }
                    token = strtok(NULL, s);
                    count++; //proceed to next tab
                }
            }
        }
      // write results to file called "result". 
        FILE* f = fopen("result", "w");
        for(int i = 1; i < macNote; i++) {
            fprintf(f, "%s\t%d\n" ,list[i].macaddress ,list[i].packetsize);
        }
        if(f != NULL) fclose(f);
      	// close file "result "
      	// 
        pid_t pid;                 // process ID returned from function fork. 
        int status; 
        switch (pid = fork()){
            case -1 :
                perror("fork()");     // process creation failed
                exit(1);
                break;
                
            case 0:              
                execv("/usr/bin/sort",Arguments); // if child processes is succesfully created
                printf("dont work\n"); // execv fails 
                _exit(1);
                break;
            default:
                wait(&status); // wait till child process completes processing
                remove("result"); // remove file "result"
                break;
                
        }
        //close packets file 
        if(file != NULL) fclose(file);
    }
    else{
        FILE* ven= fopen(fileoui,"r"); //open the vendor file
        if(ven==NULL){ //check if the input is null
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
        while(fgets(buff2,BUFSIZ,file)!=NULL){ //loops through each line in the file
            int noOftabs=0; //denotes what column we are on
            int len = strlen(buff2); //length of the current line 
            int k=0; //loop variable
          	/*
            These boolean variables check if there is a broadcast address in the current line.
            */
            bool broadcast1=false;
            bool broadcast2=false;
            /* 
            macpos gives the index of a distinct MAC address in the struct, if it exists
          	if it does not exit, this will be -1
            */
            int macpos; 
            for(int i=0;i<len;i++){ //for each character in each line
                char mac[10]; //current MAC address(es)
                char packets[5]; //current packetsize
                if(noOftabs<1 && buff2[i]!='\t') continue; //ignores the first column
                if(buff2[i]=='\t') {noOftabs++; continue;} //increases the column count if we reach a tab
              	/*
                IF we are looking at the receiving packets. This block copies the strings found in the appropriate column
                into the temporary arrays defined earlier.
                */
                if(strcmp(input,"r") == 0) {
                    if(noOftabs==1){ 
                        char check[9];
                        memcpy(check,&buff2[i],8);
                        check[8]='\0';
                        i+=16;
                        if(mystrcmp(check,VENBROAD))broadcast2=true; //if we find a broadcast address
                    }
                    if(noOftabs==2){
                        memcpy(mac,&buff2[i],8);
                        mac[8]='\0';
                        i=i+16;
                        //if broadcast address encountered.
                        if(mystrcmp(mac,VENBROAD)) broadcast1=true; //if we find a broadcast address
                    }
                }
              	/*
                IF we are looking at the transmitting packets. This block copies the strings found in the appropriate column
                into the temporary arrays defined earlier.
                */
                else if(strcmp(input,"t") == 0) {
                    if(noOftabs==1){
                        memcpy(mac,&buff2[i],8);
                        mac[8]='\0';
                        i=i+16;
                        //if broadcast address encountered.
                        if(mystrcmp(mac,VENBROAD)) broadcast1=true; //if we find a broadcast address
                    }
                    if(noOftabs==2){
                        char check[9];
                        memcpy(check,&buff2[i],8);
                        check[8]='\0';
                        i+=16;
                        if(mystrcmp(check,VENBROAD))broadcast2=true; //if we find a broadcast address
                    }
                }
              // if no broadcast addresses are fouund 
                if(broadcast1==false&& broadcast2==false ){
                  	// find index of OUI vendor name for array Vendor[]
                    index=findoui(mac,macVen,line);
                    //if no oui exists.
                    if(index==-1) strcpy(mac,"??:??:??"); //make the OUI unknown
                    macpos=isDistinct(mac); //check if the current OUI is distinct
                    if(index!=-1){
                        strcpy(list[macpos].vendor,Vendor[index]); //assign the correct name
                    }else{
                        strcpy(list[macpos].vendor,"UNKNOWN-VENDOR") ; //if OUI is unknown 
                    }
                }else {break;}
            	//Sums up all the packets for distinct macaddresse./ Sums up all the packets for distinct macaddresse.
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
      // write results to file called "result"
        FILE* f = fopen("result", "w");
        int i;
        if(strcmp(input,"t") == 0) i = 0;
        else i = 1;
      // copy all info from struct to file
        while(i<macNote){
            fprintf(f,"%s\t%s\t%i\n",list[i].macaddress,list[i].vendor,list[i].packetsize);
            i++;
        }  
        for(int i=0;i<line;i++){
            // free malloc'd memory once done for vendor names and vendor oui
            free(macVen[i]);
            free(Vendor[i]);
        }
        if(f != NULL) fclose(f); // close file pointer for  "results" file 
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
                wait(&status); // wait for child process to finish
                remove("result"); // remove file "result"  
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
    if(argc==3) {
        analyze(argv[1],argv[2],"FILENA");
        exit(EXIT_SUCCESS);
    }
    else if(argc==4){
        analyze(argv[1],argv[2],argv[3]);
    }
    else {
        fprintf(stderr,"Provide input in ./program what packets ouifile(optional)\n" );
    }
    return 0;
}
//end