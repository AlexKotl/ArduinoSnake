 unsigned char i;
 unsigned char j; 
/*Port Definitions*/
int Max7219_pinCLK = 10; // old 10
int Max7219_pinCS = 11; // old 9
int Max7219_pinDIN = 12; // old 8

int lastDirection = 1;
int snakeSpeed;
int snakeLength = 3;
int lastStepTime = 0;
int appleBlinkInterval = 100;
int appleLastBlink = 0;
int cur_time = 0;

byte matrix[8][8] = {
  {0,0,1,1,1,0,0,0},
  {0,1,0,0,0,1,0,0},
  {1,0,1,0,1,0,1,0},
  {1,0,0,0,0,0,1,0},
  {1,1,0,0,0,1,1,0},
  {1,0,1,1,1,0,1,0},
  {0,1,0,0,0,1,0,0},
  {0,0,1,1,1,0,0,0}
};

byte apple[2] = {5,5};

byte snake[64][2];



void Write_Max7219_byte(unsigned char DATA) 
{   
            unsigned char i;
	    digitalWrite(Max7219_pinCS,LOW);		
	    for(i=8;i>=1;i--)
          {		  
             digitalWrite(Max7219_pinCLK,LOW);
             digitalWrite(Max7219_pinDIN,DATA&0x80);// Extracting a bit data
             DATA = DATA<<1;
             digitalWrite(Max7219_pinCLK,HIGH);
           }                                 
}

void Write_Max7219(unsigned char address,unsigned char dat)
{
        digitalWrite(Max7219_pinCS,LOW);
        Write_Max7219_byte(address);           //address，code of LED
        Write_Max7219_byte(dat);               //data，figure on LED 
        digitalWrite(Max7219_pinCS,HIGH);
}


void Init_MAX7219(void)
{
 Write_Max7219(0x09, 0x00);       //decoding ：BCD
 Write_Max7219(0x0a, 0x00);       //brightness 
 Write_Max7219(0x0b, 0x07);       //scanlimit；8 LEDs
 Write_Max7219(0x0c, 0x01);       //power-down mode：0，normal mode：1
 Write_Max7219(0x0f, 0x00);       //test display：1；EOT，display：0
}

void setup()
{
  
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  delay(50);
  Init_MAX7219();
  Serial.begin(9600);
  randomSeed(analogRead(0));
  
  initSnake();
  generateApple();
}

void initSnake() {
  snake[0][0] = 4; snake[0][1] = -1;
  snake[1][0] = 4; snake[1][1] = 0;
  snake[2][0] = 4; snake[2][1] = 1;
  snakeLength = 3;
  snakeSpeed = 300;
  for (int i=3; i<64; i++) {
    snake[i][0] = snake[i][1] = -1;
  }
}

void gameOver() {
  //memset(snake,0,sizeof(snake));
  lastDirection = 1;
  clearMatrix();
  matrix[4][4] = 1;
  matrix[5][5] = 1;
  matrix[3][3] = 1;
  matrix[5][3] = 1;
  matrix[3][5] = 1;
  matrix[6][6] = 1;
  matrix[2][2] = 1;
  matrix[2][6] = 1;
  matrix[6][2] = 1;
  displayMatrix();
  delay(2000);
  //clearMatrix();
  initSnake();
  generateApple();
}

void displayMatrix() {
  char v = 0;
  for (i=0; i<8; i++) {
    v = 0;
    for (j=0; j<8; j++) {
      v = v << 1;
      v += matrix[i][j];
    }
    Write_Max7219(i+1,v);
  }
}

void applySnakeToMatrix() {
 for (i=0; i<sizeof(snake)/2; i++) {
   if (snake[i][0] != -1) matrix[snake[i][0]][snake[i][1]] = 1;
 } 
 // apply apple
 if (cur_time-appleLastBlink>appleBlinkInterval) {
   appleLastBlink = cur_time; 
   matrix[apple[0]][apple[1]] = 1 - matrix[apple[0]][apple[1]];

  }
  //matrix[apple[0]][apple[1]] = 1;
}

void clearMatrix() {
  for (i=0; i<8; i++) {
    for (j=0; j<8; j++) {
      matrix[i][j] = 0;
    }
  } 
}

void moveSnake(int dir) {
  // up=119, down=115, left=97, right=100
  int next_x = 0, next_y = 0;
  
  switch (dir) {
   case 1: 
     next_x = snake[snakeLength-1][0];
     next_y = snake[snakeLength-1][1]+1;
   break; 
   case 2: 
     next_x = snake[snakeLength-1][0]+1;
     next_y = snake[snakeLength-1][1];
   break;
   case 3: 
     next_x = snake[snakeLength-1][0];
     next_y = snake[snakeLength-1][1]-1;
   break;
   case 4: 
     next_x = snake[snakeLength-1][0]-1;
     next_y = snake[snakeLength-1][1];
   break;
  }
  
  lastDirection = dir;
  
  // eat apple
  if (next_x==apple[0] && next_y==apple[1]) { 
    generateApple();    
    snakeLength++;
    snakeSpeed -= 10;
  }
  
  else {
    for (i=0; i<snakeLength-1; i++) { // move stack
      snake[i][0] = snake[i+1][0];  
      snake[i][1] = snake[i+1][1];
    }
  }

  snake[snakeLength-1][0] = next_x; // here apple come bacK!!!
  snake[snakeLength-1][1] = next_y;
  
  if (snake[snakeLength-1][0]>=8 || snake[snakeLength-1][1]>=8
    || matrix[snake[snakeLength-1][0]][snake[snakeLength-1][1]]==1
  ) gameOver();

}

void generateApple() {
  do {
    apple[0] = random(7);
    apple[1] = random(7);
  } while (matrix[apple[0]][apple[1]]==1);
  
}

void loop()
{ 

  
  int axis_x = analogRead(A1);
  int axis_y = analogRead(A2);
  if (axis_y>1000 && lastDirection!=1) lastDirection = 3;
  if (axis_y<100 && lastDirection!=3) lastDirection = 1;
  if (axis_x>1000 && lastDirection!=4) lastDirection = 2;
  if (axis_x<100 && lastDirection!=2) lastDirection = 4;
  
  
  
  cur_time = millis();
  if (cur_time-lastStepTime>snakeSpeed) {
    lastStepTime = cur_time;
    moveSnake(lastDirection);
  }
  clearMatrix();
  applySnakeToMatrix();
  
  displayMatrix();

}
