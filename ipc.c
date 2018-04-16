#include "ipc.h"

unsigned char quit = 0;
void user_signal1(int sig){ quit = 1;}
void
input_proc(char* shm){
    int fd_key;
    int size;
    int val;
    struct input_event ev[BUF_SIZE];

    int fd_switch;
    unsigned char push_sw_buff[MAX_SWITCH];
    unsigned char prev_sw_buff[MAX_SWITCH];

    int i;

    printf("PROCESS IN\n");
    size = sizeof(struct input_event);

    if((fd_key = open(KEY_DEVICE,O_RDONLY | O_NONBLOCK))<0){
        printf("Device open error for KEY\n");
        close(fd_key);
    }
    if((fd_switch = open(SWITCH_DEVICE, O_RDONLY | O_NONBLOCK))<0){
        printf("Device open error for SWITCH\n");
        close(fd_switch);
    }
    (void)signal(SIGINT,user_signal1);

    while(!shm[0]){
        if(read(fd_key, ev, size* BUF_SIZE)>size){
            val = ev[0].value;
            if(val==KEY_PRESS){
                if(ev[0].code==158){ shm[0] = 1; printf("1\n");}
                else if(ev[0].code==114) {shm[1] = 1; printf("2\n");}
                else if(ev[0].code==115) {shm[2] = 1; printf("3\n");}
            }
        }
        read(fd_switch,&push_sw_buff,MAX_SWITCH);
        for(i=0;i<MAX_SWITCH;i++){
            if(push_sw_buff[i] == 1 && prev_sw_buff[i] == 0){
               shm[3+i] = push_sw_buff[i];
            }
            prev_sw_buff[i] = push_sw_buff[i];
        }
        usleep(30000);
    }
    close(fd_switch);
    close(fd_key);
}

void outdev_open(int* fd_fnd, int* fd_lcd, int* fd_dot, int* fd_led,int* fd_mot){
    if((*fd_fnd=open(FND_DEVICE,O_RDWR))<0){
        printf("Device open error for FND\n");
    }
    if((*fd_lcd=open(LCD_DEVICE,O_RDWR))<0){
        printf("Device open error for LCD\n");
    }
    if((*fd_dot=open(DOT_DEVICE,O_RDWR))<0){
        printf("Device open error for DOT\n");
    }
    if((*fd_led=open(LED_DEVICE,O_RDWR | O_SYNC))<0){
        printf("Device open error for LED\n");
    }
    if((*fd_mot=open(MOT_DEVICE,O_RDWR))<0){
        printf("Device open error for MOT\n");
    }
}

void out_draw(int fd_fnd, int fd_dot, char* shm){
    static int row=0, col=64;
    static char dot[10] ={0};
    char fnd[4] = {0};
    static clock_t t=0;
    static int counter = 0;
    static int mode = 0;
    static int flag=0;
    int i;
    int temp;

    if(shm[1]||shm[2]){ 
        memset(dot,0,sizeof(dot));
        row = 0; col=64; 
        counter=0;
        mode=0;
    }
    if(shm[3]){
        memset(dot,0,sizeof(dot));
        row = 0; col=64;
        counter+=1;
    }
    if(shm[5]) {
        if(flag%2){dot[row] ^= col; flag=0;}        
        mode^=1; counter ++;
    }
    if(shm[7]){ dot[row] ^= col; counter++;}
    if(shm[9]){ memset(dot,0,sizeof(dot)); counter++;}
   
    if(shm[4]){
        if(row>0){
            if(flag%2){dot[row] ^= col; flag=0;}
            row--; 
        }
        counter++;
    }
    if(shm[6]){
        if(col<64){
            if(flag%2){dot[row] ^= col; flag=0;} 
            col<<=1; 
        }
        counter++;
    }
    if(shm[8]){
        if(col>1){
            if(flag%2){dot[row] ^= col; flag=0;} 
            col>>=1; 
        }
        counter++;
    }
    if(shm[10]){
        if(row<9){
            if(flag%2){dot[row] ^= col; flag=0;} 
            row++;
        }
        counter++;
    }

    if(shm[11]){
        for(i=0;i<10;i++){
            dot[i] = dot[i]^127;
        }
        counter++;
    }
    
    if(!mode)
        if((clock() - t)>SEC/2){
            flag++;
            dot[row]^=col;
            t=clock();
        }
    temp = counter;
    for(i=3;i>=0;--i){
        fnd[i] = temp%10;
        temp/=10;
    } 
    write(fd_fnd,fnd,4);
    write(fd_dot,dot,10);
}

void out_text(int fd_fnd, int fd_lcd, int fd_dot, char* shm){
    char fnd[4]={0};
    char lcd[MAX_LCD];
    static int board_idx=0;
    static int cursor=-1;
    static int mode=0;
    static int counter=0;
    char* textboard[MAX_SWITCH] = {".QZ","ABC","DEF","GHI","JKL","MNO","PRS","TUV","WXY"};
    char dot[2][10]={{0x1c,0x36,0x63,0x63,0x63,0x7f,0x7f,0x63,0x63,0x63},
                    {0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}};
    int i;
    int temp;
    
    if(cursor==-1){
        memset(lcd, 0 , sizeof(lcd));
        write(fd_lcd,lcd,32);
        cursor=0; 
    }
    if(shm[1] || shm[2]){ cursor=-1; counter=0; mode = 0; return;}
    if(shm[4] && shm[5]){ cursor=-1; counter++; return;}                        
    if(shm[7] && shm[8]){ mode^=1; counter++; return;}                         
    if(shm[10] && shm[11]){ cursor++; lcd[cursor]=' '; counter++;}              
    else if(!mode){
        if(lcd[cursor]==0){
            for(i=0;i<MAX_SWITCH;i++)
                if(shm[i+3])  lcd[cursor] = textboard[i][0];
        }
        for(i=3;i<=11;i++){
            if(shm[i]){
                if(('Q'==lcd[cursor] || 'Z'==lcd[cursor] || '.' == lcd[cursor]) && i==3){
                    board_idx = (board_idx+1)%3;
                    lcd[cursor] = textboard[0][board_idx];
                }
                else if(textboard[i-3][0]<=lcd[cursor] && lcd[cursor]<=textboard[i-3][2] && i!=3){
                    board_idx = (board_idx+1)%3;
                    lcd[cursor] = textboard[i-3][board_idx];
                }
                else{
                    cursor++;
                    board_idx = 0;
                    lcd[cursor] = textboard[i-3][0];
                }
                counter++;
            }
        }
    }
    else{
        for(i=3;i<=11;i++){
            if(shm[i]){
                if(lcd[cursor]==0) 
                    lcd[cursor] = '1'+i - 3;
                else
                    lcd[++cursor] ='1'+ i - 3;
                counter++;
            }
        }
    }
    if(cursor>=8){
        cursor--;
        for(i=0;i<8;i++){
            lcd[i]=lcd[i+1];
        }
    }
    temp = counter;
    for(i=3;i>=0;--i){
        fnd[i] = temp%10;
        temp/=10;
    }
    write(fd_dot, dot[mode], 10);
    write(fd_fnd, fnd, 4);
    write(fd_lcd, lcd, 8);
}

void out_spin(int fd_dot, int fd_mot,char* shm){
    static int row=0, col=64;
    static int speed=0;
    int direction;
    static char dot[10]={64,126,0,63,0,126,0,63,0,126};
    static unsigned char motor_state[3]={0};
    static clock_t t;

    if(shm[1]||shm[2]){
        row = 0; col=64;
        speed = 0;
        dot[0] = 64;    dot[1] = 126;  dot[2] = 0;    dot[3] =63;   dot[4] = 0;
        dot[5] = 126;   dot[6] = 0;    dot[7] = 63;   dot[8] = 0;   dot[9] = 126;
    }
    if(shm[8]){ 
        if(speed>-4)
            speed--;
    }
    if(shm[6]){
        if(speed<4)
            speed++;
    }

    if(speed==0) motor_state[0] = 0;
    else motor_state[0] = 1;
    if(speed<=0) motor_state[1] = 0;
    else motor_state[1] = 1;
    if(speed<0) speed*=-1;

    motor_state[2] = 250>>speed;
    if(clock()-t>(SEC>>speed)){
        dot[row]^=col;
        if(motor_state[1]==0 && speed){
            if(row==9 && col==1){row=0;col=64;} 
            if(col&1){
                if(row&2) col<<=1;
                else row++;
            }
            else if(col&64){
                if(row&2) row++;
                else col>>=1;
            }
            else{
                if(row&2) col<<=1;
                else col>>=1;
            }
        }
        if(motor_state[1]==1 && speed){
            if(row==0 && col==64){row=9;col=1;} 
            if(col&1){
                if(row&3) row--;
                else col<<=1;
            }
            else if(col&64){
                if(row==3 || row==4 || row==7||row==8) row--;
                else col>>=1;
            }
            else{
                if(row&2) col>>=1;
                else col<<=1; 
            }
        }
        dot[row]^=col;
        t=clock();    
    }
    write(fd_dot, dot, 10);
    write(fd_mot, motor_state,3);
    if(motor_state[1]==0) speed*=-1;
}

void out_counter(int fd_fnd,char* led_addr ,char* shm){
    char fnd[4] = {0};
    static int counter = 0;
    static int bidx = 0;
    char base[4] = {10,8,4,2};
    char led[4] = {64,32,16,128};
    int temp;
    int i;

    if(counter == 0) *led_addr=64;
    *led_addr=led[bidx];
    if(shm[1]||shm[2]){ counter=0; bidx=0;}
    if(shm[3]){
        bidx = (bidx+1)%4;
        printf("BASE : %d\n",base[bidx]);
    }
    if(shm[4])counter+=base[bidx]*base[bidx];
    if(shm[5])counter+=base[bidx]; 
    if(shm[6])counter+=1; 
    temp = counter;
    for(i=3;i>=0;--i){
        fnd[i] = temp%(int)base[bidx];
        temp/=(int)base[bidx];
    }
    fnd[0] = 0;
    write(fd_fnd,&fnd,4); 
}

void out_clock(int fd_fnd,char* led_addr,char* shm/*,time_t rawtime, struct tm* timeinfoi*/){
    char fnd[4]={0};
    static int clock_edit=0;
    static char flag=0,init=0;
    static clock_t s, t;
    time_t rawtime=time(NULL);
    static struct tm timeinfo;
    if(!init) {timeinfo=*localtime(&rawtime); init=1;}

    *led_addr|=128;
    time(&rawtime);
    if(shm[1]||shm[2]) timeinfo = *localtime(&rawtime);
    if(shm[3]){
        flag =0;
        clock_edit^=1;
        printf("clock edit mode\n");
    }
    if(clock_edit){
        if(clock()-s>SEC){
            if(flag==0){ 
                *led_addr^=16;
                flag=1;
            }
            else
                *led_addr^=48;
            s=clock();
        }
        if(shm[4]) {
            timeinfo = *localtime(&rawtime);            
        }                                               
        if(shm[6]) timeinfo.tm_min += 1;                
        if(shm[5]) timeinfo.tm_hour += 1;               
    }
    else if(clock()-t>SEC){
        *led_addr&=128;
        timeinfo.tm_sec+=1;
        t = clock();
        printf("%s",asctime(&timeinfo)); 
    }
                
    timeinfo.tm_hour += (timeinfo.tm_min)/60;
    timeinfo.tm_hour %=24;
    timeinfo.tm_min +=(timeinfo.tm_sec)/60;
    timeinfo.tm_min %=60;
    timeinfo.tm_sec %=60;
    fnd[0] = timeinfo.tm_hour / 10;    fnd[1] =  timeinfo.tm_hour % 10;
    fnd[2] = timeinfo.tm_min / 10;     fnd[3] =  timeinfo.tm_min % 10;
                
    write(fd_fnd,&fnd,4);    
}

void out_maze(int fd_fnd,int fd_dot,char* shm){
    char maze[10]={4,45,32,86,67,43,14,32,55,64};
    static char visit[10]={4,45,32,86,67,43,14,32,55,64};
    char fnd[4] = {0};
    static int col=64, row=0;
    static int mode=0;
    static clock_t t=0;
    static char init = 0;
    static int counter = 0;
    char chg = 0;
    int i, temp;
    static int speed = 1;
    
    if(shm[1] || shm[2] || shm[3]){
        memcpy(visit,maze,10);
        col=64; row = 0;
        top = -1;
        init = 0;
        counter=0;
        speed = 1;
        if(shm[3]){
            mode=(mode+1)%2;
            if(!mode) printf("MANUAL\n");
            else printf("DFS\n");
        }
        else mode = 0;
    }

    if(col==1 && row ==9){
        memcpy(visit,maze,10);
        col=64; row = 0;
        top=-1;
        counter = 0;
    }
    /* Manual mode */
    if(!mode){
        if(shm[8] && !(maze[row]&col>>1)){
            if(col>1){col>>=1; counter++;}
        }
        else if(shm[10] && !(maze[row+1]&col)){
            if(row<9){row++; counter++;}
        }
        else if(shm[6] && !(maze[row]&col<<1)){
            if(col<64){col<<=1; counter++;}
        }
        else if(shm[4] && !(maze[row-1]&col)){
            if(row>0){row--; counter++;}
        }
    }
    /* DFS */
    else if(mode == 1){
        if(shm[6] && speed > 1)
            speed--;
        if(shm[8] && speed < 4)
            speed++;
        if(clock()-t>SEC>>speed){
            t=clock();
            if(!init){
                top++;
                stack[top].row = row;
                stack[top].col = col;
                init = 1;
                goto write;
            }
            if(!(visit[row]&(col>>1))&&col>1&&stack[top-1].col!=col>>1){
                    col>>=1;
                    chg=1;
            }
            else if(!(visit[row+1]&col)&&row<9&&stack[top-1].row!=row+1){
                    row++;
                    chg=1;
            }
            else if(!(visit[row]&(col<<1))&&col<64&&stack[top-1].col!=col<<1){
                    col<<=1;
                    chg=1;
            }
            else if(!(visit[row-1]&col)&&row>0&&stack[top-1].row!=row-1){
                    row--;
                    chg=1;
            }
            if(chg==0){
                visit[row]^=stack[top].col;
                top--;
                row = stack[top].row;
                col = stack[top].col;
                counter++;
            }
            else{
                top++;
                stack[top].row = row;
                stack[top].col = col;
                counter++;
            }
        }
    }
    temp = counter;
    for(i=3;i>=0;--i){
        fnd[i] = temp%10;
        temp/=10;
    }
    /* BFS */
    write(fd_fnd, fnd, 4);
write:
    maze[row]^=col; 
    write(fd_dot, maze, 10);
}

void initialize(int fd_fnd, int fd_lcd,int fd_dot, char *led_addr,int fd_mot){
    char fnd[4]={0};
    char lcd[MAX_LCD]={0};
    char dot[10]={0};
    char motor[3] = {0};
    write(fd_dot, dot, 10);
    write(fd_fnd, fnd, 4);
    write(fd_lcd, lcd, 32);
    write(fd_mot, motor,3);
    *led_addr = 0;
    top = -1;
}

void
output_proc(char* shm){
    int mode;
    int fd_fnd;
    int fd_lcd;
    int fd_dot;
    int fd_led;
    int fd_mot;
    /////////////////////////////////////
    unsigned long *fpga_addr = 0;
    unsigned char *led_addr = 0;
    unsigned char led;
    //////////////////////////////////////
    printf("PROCESS OUT\n");
    outdev_open(&fd_fnd, &fd_lcd, &fd_dot, &fd_led,&fd_mot);
    
    /* initialize */
    mode = 0;
    led = 128;
    fpga_addr = (unsigned long*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd_led, FPGA_BASE_ADDRESS); 
    led_addr = (unsigned char*)((void*)fpga_addr+LED_ADDR);
    *led_addr = led;

    while(!shm[0]){
        /* CHANGE MODE */
        if(shm[1]){ 
            mode = (mode+5)%6;
            printf("mode : %d\n",mode);
        }
        if(shm[2]){
            mode = (mode+1)%6;
            printf("mode : %d\n",mode);
        }
        if(shm[1]||shm[2]){
            initialize(fd_fnd, fd_lcd, fd_dot,led_addr,fd_mot);
        }
        switch(mode){
            case CLOCK:
                out_clock(fd_fnd,led_addr,shm);
                break;
            case COUNTER:
                out_counter(fd_fnd,led_addr,shm);
                break;
            case TEXT:
                out_text(fd_fnd,fd_lcd,fd_dot, shm);
                break;
            case BOARD:
                out_draw(fd_fnd,fd_dot,shm);
                break;
            case SPIN:
                out_spin(fd_dot,fd_mot,shm);
                break;
            case 5:
                out_maze(fd_fnd,fd_dot,shm);
                break;
        }
        shm[1] = 0; shm[2] = 0;
        shm[3] = 0; shm[4] = 0;
        shm[5] = 0; shm[6] = 0;
        shm[7] = 0; shm[8] = 0;
        shm[9] = 0; shm[10] = 0;
        shm[11] = 0; 
        usleep(2000);
    }
    initialize(fd_fnd, fd_lcd, fd_dot,led_addr,fd_mot); 
    close(fd_fnd);
    close(fd_lcd);
    close(fd_dot);
    close(fd_mot);
    munmap(led_addr, 4096);
    printf("Free Resources\n");
    printf("Program exit\n");
}


int main(){
    pid_t pid_in=0, pid_out=0;
    key_t key = 0;
    int shm_id;
    char* shm;
    int status;
    sem_t *sema;
    
    /* SHARED MEM ALLOCATION */
    shm_id = shmget(key,128,IPC_CREAT|0644);
    if(shm_id<0)
        printf("shared mem get error for MODE\n");
    shm = (char*)shmat(shm_id,(char*)NULL,0);
    
    /* PROCESS FORK */
    pid_in = fork();
    if(pid_in<0){
        printf("fork failure\n");
    }
    if(pid_in){
        pid_out = fork();
        if(pid_out<0){
            printf("fork failure\n");
        }
    }

    if(!pid_in){
        pid_out=-1;
        input_proc(shm);
    }
    if(!pid_out){
        output_proc(shm);
    }
    if(pid_in && pid_out){
        pid_in = wait(&status);
        printf("INPUT PROCESS EXIT STATUS : %d\n",status);
        pid_out= wait(&status);
        printf("OUTPUT PROCESS EXIT STATUS : %d\n",status);
    }
    shmdt(shm);
   
    return 0;
}


