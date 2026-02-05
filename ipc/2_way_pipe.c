#include<stdio.h>
#include<unistd.h>
#include<string.h>

int main(){
    int pipe1[2]; // pipe from parent to child
    int pipe2[2]; // pipe from child to parent

    pipe(pipe1);
    pipe(pipe2);

int pid = fork();
if(pid==0){
    close(pipe1[1]);
    close(pipe2[0]);

    char buff[25];
    read(pipe1[0], buff,sizeof(buff)); // pipe arguments
    printf("child read :%s\n" ,buff );

    char reply[] = "hello parent";
    write(pipe2[1], reply , strlen(reply)+1);    // +1 for null character or EOF
    close(pipe1[0]);
    close(pipe2[1]);
}else{
    char message[] = "hello child";
    write(pipe1[1], message , strlen(message)+1);
    
    char buff[25];
    read(pipe2[0], buff , sizeof(buff));
    printf("parent read :%s\n", buff);

    close(pipe1[1]);
    close(pipe2[0]);

}
return 0;
}