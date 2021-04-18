#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_LINE 80

int main(void)
{ char input[MAX_LINE]; char *args[MAX_LINE/2]; char *pipe_first[MAX_LINE/4]; char *pipe_second[MAX_LINE/4];
int fd[2];
while(1){
/* 명령어를 입력 받음 */
printf("osh> ");
fflush(stdout);
if (!fgets(input,MAX_LINE,stdin)) break;
input[strlen(input)-1] = '\0';
/* exit을 입력받으면 멈춤 */
if (!strcmp(input,"exit")) break; /* 띄어쓰기 단위로 문자열을 나눠서 args배열에 저장 */ char *ptr = strtok(input," ");
int index = 0; while (ptr != NULL){
args[index] = ptr; ptr = strtok(NULL," ");
index++;
}
args[index] = NULL; // 배열의 끝에 NULL을 추가
/* 배열의 마지막에 &이 있는지 확인 */
int background = 0;
if (strcmp(args[index-1],"&") == 0){
args[index-1] = NULL; background = 1;
index--;
}
/* 파이프 명령어인지 확인 */
int pipe_num = 0;
if (index >= 3 && (strcmp(args[1],"|") == 0 || strcmp(args[2],"|") == 0)) {
/* 명령어 | 명령어 */
if (index == 3){
pipe_first[0] = args[0];
pipe_first[1] = NULL; pipe_second[0] = args[2];
pipe_second[1] = NULL;
}
/* 명령어+옵션 | 명령어+옵션*/
else if (index == 5){
pipe_first[0] = args[0];
pipe_first[1] = args[1];
pipe_first[2] = NULL; pipe_second[0] = args[3];
pipe_second[1] = args[4];
pipe_second[2] = NULL;
}else {
/* 명령어 | 명령어+옵션 */
if(!strcmp(args[1],"|")){
pipe_first[0] = args[0];
pipe_first[1] = NULL; pipe_second[0] = args[1];
pipe_second[1] = args[2];
pipe_second[2] = NULL;
}
/* 명령어+옵션 | 명령어 */
else if(!strcmp(args[2],"|")){
pipe_first[0] = args[0];
pipe_first[1] = args[1];
pipe_first[2] = NULL; pipe_second[0] = args[3];
pipe_second[1] = NULL;
}
}
pipe_num = 1;
}
int status;
int pid = fork();
/* fork 실패시 */
if (pid < 0){
fprintf(stderr,"Fork failed"); return 1;
}
/* 자식 프로세스 일 때 */
else if (pid == 0){
/* 명령어 or 명령어-옵션 일때 */
if (index == 1 || index == 2){
args[index]=NULL;
}
/* ">" 명령어가 포함되어 있을 때 */
else if (strcmp(args[index-2],">") == 0){
int fd = open(args[index-1],O_CREAT|O_RDWR,0644);
if (fd < 0){
printf("Redirection Error\n");
exit(0);
}
dup2(fd,1); //STDOUT_FILENO
args[index-2] = NULL; close(fd);
}
/* "<" 명령어가 포함되어 있을 때 */
else if (strcmp(args[index-2],"<") == 0){
int fd = open(args[index-1],O_RDONLY);
if (fd < 0){
printf("Redirection Error\n");
exit(0);
}
dup2(fd,0); //STDIN_FILENO
args[index-2] = NULL; close(fd);
}
/* "|" 명령어가 포함되어 있을 때 */
if (pipe_num){
pipe(fd);
int pid1 = fork();
if (pid1 == 0){
dup2(fd[0],0); //STDIN_FILENO
close(fd[1]);
execvp(pipe_second[0],pipe_second);
}else {
dup2(fd[1],1); //STDOUT_FILENO
close(fd[0]);
execvp(pipe_first[0],pipe_first);
}
}
execvp(args[0],args);
}
else{
if (!background) waitpid(pid,&status,0); // 백그라운드로 돌아가는 것이 아니면 자식프로세스를 기다린다
else printf("background\n");
}
}
}