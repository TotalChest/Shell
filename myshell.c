#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

void conv(int r,char **kom,int *mas3);
char* pars(char* strok,int *p,int* stat);
void ctrlc(int s);
void ctrlca(int s);
int get(char** kom,int r,char* lex,int *mes);
void komand(int r,char **kom);

int main(int argc,char **argv){
    
    char **mas=NULL,**kom=NULL;
    char *ss=NULL,*strok=NULL, c, *buf;
    int dfon,fonpid,mas3[16],pwd,fd[2],pwdpid,p1=0,s=0,k=0,p=0,i=0,r=0,stat=1,mes,d1,d2,d3,std0,std1,r3;
    
    signal(SIGINT,ctrlc);
    signal(SIGALRM,ctrlca);
    
    //Аргументы
    if ((argc!=1)&&(argc!=2)){
        printf("\033[1;31mНеправильное количество аргументов SHELL!\033[01;0m\n");
        return 1;
    }
    
    //Ввод из файла
    if (argc==2){
        int d;
        d=open(argv[1],O_RDONLY);
        if (d!=-1){
            dup2(d,0);
            close(d);
        }
        else
        {
            printf("\033[1;31mТакого файла не существует!\033[01;0m\n");
            exit(1);
        }
    }
    
    //Глобальный цикл до конца файла
    c='\0';
    while(c!=EOF){  
        
        //Приглашение ко вводу
        pipe(fd);
        if((pwdpid=fork())==0){
            dup2(fd[1],1);
            execlp("pwd","pwd",NULL);   
            exit(0);
        }
        buf=(char*)malloc(100*sizeof(char));
        pwd=read(fd[0],buf,100);
        waitpid(pwdpid,NULL,0);
        buf[pwd-1]='\0';
        printf("\033[01;34m[%s]>>>\033[01;0m ",buf);
        free(buf);
        close(fd[0]);
        close(fd[1]);
        
        
        //Формирование строки
        s=0;
        c=getchar();
        while((c!='\n')&&(c!=EOF)){
            strok=(char*)realloc(strok,(s+1)*sizeof(char)); 
            if (strok==NULL){
                printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                exit(1);
            }
            strok[s]=c;
            s++;
            c=getchar();
        }
        strok=(char*)realloc(strok,(s+1)*sizeof(char)); 
        if (strok==NULL){
            printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
            exit(1);
        }
        strok[s]='\0';
        
        if (strok[0]=='\0')
            continue;
        
        //Парсинг строки
        k=0;
        p1=0;
        stat=1;
        while (stat){
            ss=NULL;
            ss=pars(strok,&p1,&stat);
            if ((stat==0)||(stat==2)){
                break;
            }
            mas=(char**)realloc(mas,(k+1)*sizeof(char*));
            if (mas==NULL){
                printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                exit(1);
            }
            mas[k]=(char*)malloc((strlen(ss)+1)*sizeof(char));
            if (mas[k]==NULL){
                printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                exit(1);
            }
            strcpy(mas[k],ss);
            free(ss);
            k++;
        }
        
        if (stat==2){
                    for(i=0;i<k;i++)
            free(mas[i]);
            free(mas);
            mas=NULL;
        
            free(strok);
            strok=NULL;
            continue; 
        }  
            
        //Цикл в строке по ';'
        p=0;
        while(p<k){
             
            r=0;
            while((p<k)&&(strcmp(mas[p],";")!=0)){
                p++;
                r++;
                kom=(char**)realloc(kom,r*sizeof(char*));
                if (kom==NULL){
                    printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                    exit(1);
                }
                kom[r-1]=(char*)malloc((strlen(mas[p-1])+1)*sizeof(char));
                if (kom[r-1]==NULL){
                    printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                    exit(1);
                }
                strcpy(kom[r-1],mas[p-1]);
            }
            
            
//==============================================================================================  
            
            std0=dup(0);
            std1=dup(1);
            
            //Фоновый процесс
            if (get(kom,r,"&",&mes)){
                
                fonpid=fork();
                if(fonpid==0){
                    if(fork()==0){
                    
                    dfon=open("/dev/null",O_RDONLY);
                    dup2(dfon,0);
                    signal(SIGINT,SIG_IGN);
                    
                    printf("\033[01;33mПроцесс в фоне!\033[01;0m\n");
                    
                    for(i=0;i<(r-mes-1);i++)
                        kom[mes+i]=kom[mes+i+1];
                    r--;  
                    
                    komand(r,kom);
                    
                    printf("\033[01;33mФоновый процесс завершен!\033[01;0m\n");
                           
                    }
                    exit(0);
                }
                waitpid(fonpid,NULL,0);
            }
            else{
            
            //Перенаправление ввода-вывода
            if (get(kom,r,"<",&mes)){
                d1=open(kom[mes+1],O_RDONLY);
                dup2(d1,0);
                close(d1);
                free(kom[mes]);
                free(kom[mes+1]);
                for(i=0;i<(r-mes-2);i++)
                    kom[mes+i]=kom[mes+i+2];
                r=r-2;
            }
            
            if (get(kom,r,">",&mes)){
                d2=open(kom[mes+1],O_WRONLY|O_CREAT|O_TRUNC,0666);
                dup2(d2,1);
                close(d2);
                free(kom[mes]);
                free(kom[mes+1]);
                for(i=0;i<(r-mes-2);i++)
                    kom[mes+i]=kom[mes+i+2];
                r=r-2;
            }
            
             if (get(kom,r,">>",&mes)){
                d3=open(kom[mes+1],O_WRONLY|O_CREAT|O_APPEND,0666);
                dup2(d3,1);
                close(d3);
                free(kom[mes]);
                free(kom[mes+1]);
                for(i=0;i<(r-mes-2);i++)
                    kom[mes+i]=kom[mes+i+2];
                r=r-2;
            }
            
            
            //Конвеер
            
            if (get(kom,r,"|",&mes)){
                for(i=0;i<16;mas3[i++]=0);
                r3=1;
                mas3[0]=0;
                while (get(kom,r,"|",&mes)){
                    r3++;
                    mas3[r3-1]=mes;
                    free(kom[mes]);
                    for(i=0;i<(r-mes-1);i++)
                        kom[mes+i]=kom[mes+i+1];
                    r--;
                }
                r3++;
                mas3[r3-1]=r;
                
                conv(r3-1,kom,mas3);
                
            }
            
            else{
                
                komand(r,kom);
                
            }
            }
            
            dup2(std0,0);
            dup2(std1,1);
            
//===================================================================================================
            
            for(i=0;i<r;++i)
                free(kom[i]);
            free(kom);
            kom=NULL;
            p++;
            
            

        }
        
        
        
        
        //Восстановление пред вводом новой строки
        for(i=0;i<k;i++)
            free(mas[i]);
        free(mas);
        mas=NULL;
        
        free(strok);
        strok=NULL;
        
        
        
    }
    
    return 0;
}

char* pars(char* strok,int *p,int* stat){
    
    char t1,c1,*f=NULL;
    int r=0,i=0,r1=0;
    
    c1=strok[*p];
    
    while(c1==' '){
        c1=strok[*p+i+1];
        i++;
    }
    
    if((c1=='\0')||(c1=='#')){
        *stat=0;
        return f;
    }
    
    if((c1=='<')||(c1=='(')||(c1==')')||(c1==';')){
        f=(char*)malloc(2*sizeof(char));
        if (f==NULL){
            printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
            exit(1);
        }
        f[0]=c1;
        f[1]='\0';
        (*p)+=1+i;
        return f;
    }
        
    if ((c1=='>')||(c1=='&')||(c1=='|')){
        t1=strok[*p+1+i];
        if (t1==c1){
            f=(char*)malloc(3*sizeof(char));
            if (f==NULL){
                printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                exit(1);
            }
            f[0]=c1;
            f[1]=c1;
            f[2]='\0';
            (*p)+=2+i;
            return f;
        }
        else{
            f=(char*)malloc(sizeof(char));
            if (f==NULL){
                printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                exit(1);
            }
            f[0]=c1;
            f[1]='\0';
            (*p)+=i+1;
            return f;
        }
    }
    
    r=0; 
    r1=0; 
    while((c1!='<')&&(c1!='(')&&(c1!=')')&&(c1!=';')&&(c1!='>')&&(c1!='&')&&(c1!='|')&&(c1!=' ')&&(c1!='#')&&(c1!='\0')){
        if (c1=='"'){
            r++;
            c1=strok[i+r+*p];
            while ((c1!='"')&&(c1!='\0')){
                f=(char*)realloc(f,(r1+1)*sizeof(char));
                if (f==NULL){
                    printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                    exit(1);
                }
                f[r1]=c1;
                r++;
                r1++;
                c1=strok[i+r+*p];
            }
        }
            
        if(c1=='\0'){
            printf("\033[1;31mОшибка кавычек SHELL!\033[01;0m\n");
            *stat=2;
                }
                
        else{
            if (c1=='"'){
                r++;
                c1=strok[i+r+*p];
                if((c1=='<')||(c1=='(')||(c1==')')||(c1==';')||(c1=='>')||(c1=='&')||(c1=='|')||(c1==' ')||(c1=='#')||(c1=='\0')||(c1=='"'))
                    continue;
            }
                            
            f=(char*)realloc(f,(r1+1)*sizeof(char));
            if (f==NULL){
                printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
                exit(1);
            }
        
            f[r1]=c1;
            r++;
            r1++;
            c1=strok[i+r+*p];  
        }
    }
        
    f=(char*)realloc(f,(r1+1)*sizeof(char));
    if (f==NULL){
        printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
        exit(1);
    }
    f[r1]='\0';
        
    (*p)+=i+r;
    return f; 
}

void ctrlc(int s){
    signal(SIGINT,SIG_DFL);
    alarm(1);
}

void ctrlca(int s){
    signal(SIGINT,ctrlc);
}

int get(char** kom,int r,char* lex,int* mes){
    int f=0,i;
    for(i=0;i<r;i++)
        if(!strcmp(kom[i],lex)){
            f=1;
            break;
        }
    *mes=i;
    return f;
}

void conv(int r,char **kom,int *mas3){
    int pidk,pid,i=0,fd[2],p,j;
    
    pidk=fork();
    if (pidk==0){
        while(++i<r+1){
            pipe(fd);
            pid=fork();
            if(pid==0){
                char **kom3=NULL;
                if (i!=r)
                    dup2(fd[1],1);
                close(fd[1]);
                close(fd[0]);
                
                p=mas3[i]-mas3[i-1];
                kom3=(char**)malloc(p*sizeof(char*));
                for(j=0;j<p;j++)
                    kom3[j]=kom[j+mas3[i-1]]; 
                komand(p,kom3);
                exit(1);
            }

            dup2(fd[0],0);
            close(fd[1]);
            close(fd[0]);
        }
        while (wait(NULL)!=-1);
        exit(0);
    }
    waitpid(pidk,NULL,0);
}

void komand(int r,char **kom){
    
    int status,prc;
            
    //status=stat(kom);
    status=1;
    if (!strcmp(kom[0],"cd"))
        status=2;
            
            
    if(status==2){
        if (r>2)
            printf("\033[1;31mНеверное количество аргументов \"cd\"!\033[01;0m\n");
        else{
            if (r==1)
                chdir( getenv ("HOME"));
            else
                if (chdir(kom[1])!=0)
                    printf("\033[1;31mОшибка \"cd\"!\033[01;0m\n");
        }                   
    }
                     
    //Обычная команда с аргументами
    if(status==1){
        kom=(char**)realloc(kom,(r+1)*sizeof(char*));
        if (kom==NULL){
            printf("\033[1;31mОшибка памяти SHELL!\033[01;0m\n");
            exit(1);
        }
        kom[r]=NULL;
                
        prc=fork();
        if(prc==0){
            execvp(kom[0],kom);
            printf("\033[1;31mОшибка команды \"%s\"\033[01;0m\n",kom[0]);
            exit(1);
        }
        else{
            if(prc==-1){
                printf("\033[1;31mОшибка создания процесса SHELL!\033[01;0m\n");
                exit(1);
            }
            else
                waitpid(prc,NULL,0);
        }    
    }
               
}


