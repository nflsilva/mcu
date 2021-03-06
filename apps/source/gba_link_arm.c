

volatile unsigned int *DWT_CYCCNT ;
volatile unsigned int *DWT_CONTROL;
volatile unsigned int *SCB_DEMCR;


void reset_timer(){
    DWT_CYCCNT   = (volatile unsigned int *)0xE0001004;
    DWT_CONTROL  = (volatile unsigned int *)0xE0001000;
    SCB_DEMCR    = (volatile unsigned int *)0xE000EDFC;
    *SCB_DEMCR   = *SCB_DEMCR | 0x01000000;
    *DWT_CYCCNT  = 0;
    *DWT_CONTROL = 0; 
}
void start_timer(){
    *DWT_CONTROL |= 1 ;
}

void stop_timer(){
    *DWT_CONTROL = *DWT_CONTROL & 0xFFFFFFFE ; 
}

unsigned int get_cycles(){
    return *DWT_CYCCNT;
}

void bit_delay(unsigned int d) {
  reset_timer();
  start_timer();
  while(get_cycles() < d){};
}


void tx_packet16(uint16_t b) {
  // start
  digitalWrite(2, LOW);
  bit_delay(610);
  // data bits
  for(uint8_t bits=0;bits<16;bits++){
    reset_timer();
    start_timer();
    if(b & 1) {
      digitalWrite(2, HIGH);
    }
    else {
      digitalWrite(2, LOW);
    }
    b >>= 1;
    while(get_cycles() < 610){};
  }
  // stop
  digitalWrite(2, HIGH);
  bit_delay(610);
}

uint32_t rx_packet16() {
  
  uint16_t d = 0;
  uint8_t s = 0;
  uint8_t e = 0;

  // wait for sender start bit
  while(digitalRead(2)){};
  
  // start bit (low)
  bit_delay(250);
  s = digitalRead(2) ? 1 : 0;

  // data bits
  reset_timer();
  start_timer();
  for(uint8_t bits=0;bits<16;bits++){
    while(get_cycles() < 615){};
    reset_timer();
    start_timer();
    d >>= 1;
    if(digitalRead(2)) {
      d |= 0x8000;
    }
  }
  
  // stop bit (HIGH)
  bit_delay(625);
  e = digitalRead(2) ? 1 : 0;
  
  // check for start OR stop bits errors
  if(s || !e) {
    digitalWrite(13, HIGH);
  }
  else {
    digitalWrite(13, LOW);
  }
  
  return d;
}

void setup() {
  
  Serial.begin(9600);
  
  pinMode(13, OUTPUT);

  pinMode(4, INPUT);
  pinMode(3, INPUT);
  pinMode(2, INPUT);
  cli();
}

void loop() {

  uint16_t b = 0;


  pinMode(2, INPUT);
  b = rx_packet16();

  while(digitalRead(3)) {};
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  bit_delay(615);

  tx_packet16(b);

  pinMode(2, INPUT);

  while(digitalRead(3) == 0){};

  //Serial.println(b);
  //delay(1000);


  
}
