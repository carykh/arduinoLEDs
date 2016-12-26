const byte POSITIVE_PINS[8] = {2,3,4,5,6,7,8,9};
const byte NEGATIVE_PINS[8] = {A5,A4,A3,A2,A1,A0,10,13};
const byte INPUT_PINS[2] = {12,11};
const byte DIRECTIONS[5][2] = {{0,0},{0,-1},{1,0},{0,1},{-1,0}};
const int NUMBER_FONT[10] = {31599,25751,29671,29647,23497,31183,31215,29257,31727,31695};
bool grid[8][8];
bool futureGrid[8][8];
int withinFrameTimer = 0;
int withinFrameLimit = 12;
int frameTimer = 0;
int frameLimit = 100;
int changeCount = 0;
bool aPressed = false;
bool bPressed = false;
bool wasAPressed = false;
bool wasBPressed = false;
bool obstaclesHigh[2] = {false,false};
int obstacleProgress = 0;
int mode = 0;
float gameTimer = 0;
float gameSpeed = 25; // Lower = faster
int score = 0;
byte snakeGrid[8][8];
byte snakeFrontX = 0;
byte snakeFrontY = 0;
byte snakeBackX = 0;
byte snakeBackY = 0;
byte snakeGameStage = B1;
int gameIterator = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("Setup completed");
  for(int i = 0; i < 8; i++){
    pinMode(POSITIVE_PINS[i], OUTPUT);
    pinMode(NEGATIVE_PINS[i], OUTPUT);
  }
  pinMode(INPUT_PINS[0],INPUT_PULLUP);
  pinMode(INPUT_PINS[1],INPUT_PULLUP);
  setMode(0);
}
void loop() {
  aPressed = digitalRead(INPUT_PINS[1]);
  bPressed = digitalRead(INPUT_PINS[0]);
  if(aPressed && !wasAPressed){
    if(mode == 2){
      changeSnakeDire(snakeGameStage == B1);
    }
  }
  if(bPressed && !wasBPressed){
    setMode((mode+1)%3);
  }
  wasAPressed = aPressed;
  wasBPressed = bPressed;
  if(mode == 0){
    showGrid();
    withinFrameTimer++;
    if(withinFrameTimer >= withinFrameLimit){
      withinFrameTimer = 0;
      doConwayGeneration();
      frameTimer++;
      if(frameTimer >= frameLimit){
        setMode(0);
      }
      if(changeCount == 0 && frameTimer < frameLimit-5){
        frameTimer = frameLimit-5;
      }
    }
  }else if(mode == 1 || mode == 2){
    if(gameSpeed < 100){
      gameTimer++;
      if(gameTimer >= gameSpeed){
        gameTimer -= gameSpeed;
        doGameIteration();
      }
    }
    showGame();
  }
}
void setMode(int target){
  mode = target;
  if(mode == 0){
    randomizeGrid();
    frameTimer = 0;
  }else if(mode == 1){
    obstaclesHigh[0] = true;
    obstaclesHigh[1] = true;
    obstacleProgress = 3;
    setGameSpeed();
    score = 0;
  }else if(mode == 2){
    initializeSnake();
  }
}
void initializeSnake(){
  gameIterator = 0;
  for(int y = 0; y < 8; y++){
    for(int x = 0; x < 8; x++){
      snakeGrid[y][x] = B0;
    }
  }
  snakeFrontX = 3;
  snakeFrontY = 7;
  snakeBackX = snakeFrontX;
  snakeBackY = snakeFrontY;
  snakeGrid[snakeFrontY][snakeFrontX] = B1;
  for(int i = 0; i < 1; i++){
    addFood();
  }
  gameSpeed = 9;
  score = 1;
  snakeGameStage = B1;
}
void doConwayGeneration(){
  for(int y = 0; y < 8; y++){
    for(int x = 0; x < 8; x++){
      int neighborCount = getNeighborsAt(x,y);
      if(grid[y][x]){
        futureGrid[y][x] = (neighborCount == 2 || neighborCount == 3);
      }else{
        futureGrid[y][x] = (neighborCount == 3);
      }
    }
  }
  changeCount = 0;
  for(int y = 0; y < 8; y++){
    for(int x = 0; x < 8; x++){
      if(grid[y][x] != futureGrid[y][x]){
        changeCount++;
      }
      grid[y][x] = futureGrid[y][x];
    }
  }
}
int getNeighborsAt(int x, int y){
  int total = 0;
  for(int y1 = max(0, y-1); y1 <= min(7, y+1); y1++){
    for(int x1 = max(0, x-1); x1 <= min(7, x+1); x1++){
      if((y1 != y || x1 != x) && grid[y1][x1]){
        total++;
      }
    }
  }
  return total;
}
void doGameIteration(){
  if(mode == 1){
    if(obstacleProgress == 0){
      obstacleProgress = 3;
      obstaclesHigh[0] = obstaclesHigh[1];
      obstaclesHigh[1] = (random(0,2) == 0);
      score++;
      setGameSpeed();
    }else{
      obstacleProgress--;
    }
  }else if(mode == 2){
    if(snakeGameStage >= B1){
      if(!extendSnake()){
        recedeSnake();
      }
    }
  }
}
boolean extendSnake(){
    boolean result = false;
    byte frontDire = snakeGrid[snakeFrontY][snakeFrontX];
    snakeFrontX = bounded(snakeFrontX+DIRECTIONS[frontDire][0]);
    snakeFrontY = bounded(snakeFrontY+DIRECTIONS[frontDire][1]);
    if(!inBounds(snakeFrontX,snakeFrontY)){
      snakeGameStage = B0;
    }else{
      byte value = snakeGrid[snakeFrontY][snakeFrontX];
      if(value == B101){
        addFood();
        //snakeGameStage = B11-snakeGameStage; //change turning direction
        score++;
        result = true;
      }else if(value >= B1){
        snakeGameStage = B0;
      }
      snakeGrid[snakeFrontY][snakeFrontX] = frontDire;
    }
    return result;
}
boolean inBounds(int x, int y){
  return (x >= 0 && x < 8 && y >= 0 && y < 8);
}
void recedeSnake(){
  byte backDire = snakeGrid[snakeBackY][snakeBackX];
  snakeGrid[snakeBackY][snakeBackX] = B0;
  snakeBackX = bounded(snakeBackX+DIRECTIONS[backDire][0]);
  snakeBackY = bounded(snakeBackY+DIRECTIONS[backDire][1]);
}
void changeSnakeDire(boolean turnRight){
  byte s = snakeGrid[snakeFrontY][snakeFrontX];
  if(turnRight){
    snakeGrid[snakeFrontY][snakeFrontX] = (s%4)+1;
  }else{
    snakeGrid[snakeFrontY][snakeFrontX] = ((s+2)%4)+1;
  }
}
void addFood(){
  int foodX = 0;
  int foodY = 0;
  int attempts = 0;
  while((attempts == 0 || snakeGrid[foodY][foodX] >= B1) && attempts < 50){
    foodX = random(0,8);
    foodY = random(0,8);
    attempts++;
  }
  if(attempts < 50){
    snakeGrid[foodY][foodX] = B101;
  }
}
byte bounded(byte b){
  return b;//((b+B1000)%B1000);
}
void setGameSpeed(){
  gameSpeed = 56*pow(score*2.5+12,-0.5);
}
void randomizeGrid(){
  for(int y = 0; y < 8; y++){
    for(int x = 0; x < 8; x++){
      grid[y][x] = futureGrid[y][x] = (random(0,2) == 0);
    }
  }
  showGrid();
}
void showGrid(){
  for(byte pNum = 0; pNum < 8; pNum++){
     digitalWrite(POSITIVE_PINS[pNum],HIGH);
  }
  for(byte pNum = 0; pNum < 8; pNum++){
    for(byte nNum = 0; nNum < 8; nNum++){
      if(grid[pNum][nNum]){
        digitalWrite(NEGATIVE_PINS[nNum],LOW);
      }else{
        digitalWrite(NEGATIVE_PINS[nNum],HIGH);
      }
    }
    digitalWrite(POSITIVE_PINS[pNum],LOW);
    delay(1);
    digitalWrite(POSITIVE_PINS[pNum],HIGH);
    delay(1);
  }
  for(byte pNum = 0; pNum < 8; pNum++){
     digitalWrite(POSITIVE_PINS[pNum],HIGH);
  }
}
void showGame(){
  for(byte pNum = 0; pNum < 8; pNum++){
     digitalWrite(POSITIVE_PINS[pNum],HIGH);
  }
  for(byte pNum = 0; pNum < 8; pNum++){
    for(byte nNum = 0; nNum < 8; nNum++){
      bool shine = false;
      if(mode == 1){
        if(nNum%4 == obstacleProgress){
          if((obstaclesHigh[nNum/4] && (pNum == 4 || pNum == 5)) || (!obstaclesHigh[nNum/4] && (pNum == 6 || pNum == 7))){
            shine = true;
          }
        }
        if(nNum == 0 && gameSpeed < 100 && ((pNum == 5 && aPressed) || (pNum == 6 && !aPressed))){
          if(shine){ // Uh-oh, two overlapping lights, that means player and wall collided.
            gameSpeed = 101;
          }
          shine = true;
        }
        if(pNum == 1 || pNum == 2){
          shine = bitRead(score,7-nNum+8*(2-pNum));
        }
      }else if(mode == 2){
        if(snakeGameStage >= B1){
          int pX = nNum; //(12+nNum+snakeFrontX)%8;
          int pY = pNum;//(12+pNum+snakeFrontY)%8;
          byte value = snakeGrid[pY][pX];
          if(value == B101){
            shine = ((gameIterator%8 < 4) == ((pX+pY)%2 == 0));
          }else{
            shine = (value >= B1);
          }
        }else{
          int digits[2] = {score/10,score%10};
          if(pNum >= 1 && pNum < 6 && nNum%4 <= 2){
            shine = bitRead(NUMBER_FONT[digits[nNum/4]],(5-pNum)*3+(2-nNum%4));
          }
        }
      }
      if(shine){
        digitalWrite(NEGATIVE_PINS[nNum],LOW);
      }else{
        digitalWrite(NEGATIVE_PINS[nNum],HIGH);
      }
    }
    digitalWrite(POSITIVE_PINS[pNum],LOW);
    delay(1);
    digitalWrite(POSITIVE_PINS[pNum],HIGH);
    delay(1);
  }
  for(byte pNum = 0; pNum < 8; pNum++){
     digitalWrite(POSITIVE_PINS[pNum],HIGH);
  }
  gameIterator++;
}

