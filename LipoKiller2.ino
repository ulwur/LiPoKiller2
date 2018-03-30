// Lipo-Killer
//
// Version: 0.2
// 2018-03-18 By Ulf I. Karlsson
// This software is released under the GPL. 
// Board: Lipokiller2 with Adafruit Metro (mini)

const bool debug = false;

// Inputs //
const int C1 = A0;
const int C2 = A1;
const int C3 = A2;
const int C4 = A3;
const int Curr = A4;

// Loads //
const int L1 = 2;
const int L2 = 3;
const int L3 = 4;
const int L4 = 5;
const int Buzz = 12;

// Calibration Constants //
const float volt_calib[4] = {0.997, 1.002, 1.015, 0.970};
const float curr_calib = 1.00;


// Misc //
float Cells[4];  //Cells array for volatges
float Current;
int load;
int CellCount;
int CellsOK;
long Starttime;
long PrintInterval = 2000;
long LastPrint;
const float max_inbalance = 0.2;       //Max imbalande between cells
const float min_start_voltage = 3.9;   //Dont start unless this voltage
int LastLoad = 254;                    //Start value for loadbalance
const int SERIAL_BAUD_RATE = 19200;    //Adjust here the serial rate.

// Functions //

void sample_cells() {
  //Samples the cell voltage and current
  
  //Sample och mappa 0-1023 to 0-5 (ie counts to volt)
  Cells[0] = fmap(analogRead(C1), 0, 1023, 0.0, 5.0) * volt_calib[0];
  Cells[1] = fmap(analogRead(C2), 0, 1023, 0.0, 5.0) * volt_calib[1];
  Cells[2] = fmap(analogRead(C3), 0, 1023, 0.0, 5.0) * volt_calib[2];
  Cells[3] = fmap(analogRead(C4), 0, 1023, 0.0, 5.0) * volt_calib[3];
  Current  = fmap(analogRead(Curr), 0, 1023, 0.0, 10.0) * curr_calib;  //5 Volt * (0.1 / 4) Ohm  * 20x Amplification ==> 10 Amp
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max){
  //mappar int till skala och returnerar en float
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void printout (){
  //Prints status messages

  int seconds = (millis() - Starttime) / 1000;
  
	String Row = " Dchg \t" ;
	Row = Row + "Time: " + String(seconds) + ",\t" ;
	
	//Loop over analog values and add
	for (int i=0; i <= 3; i++){
		Row = Row + "Cell" + String(i+1) + ": " + String(Cells[i]) + ",\t";
	}

	//Add Current
	Row = Row + "Curr: " + String(Current) + ",\t";
	
	//Add load
	Row = Row + "Load: " + String(load) + ",\n";
	
  
  //Print to serial
  if (debug){Serial.println("Printging...");}
  Serial.println(Row);
}


// Applies loads.
void apply_load(int loads){

	if (debug){Serial.println("apply_loads: ");}
	if (debug){Serial.println(loads);}
  
	if (loads >= LastLoad){                  	//Same or larger load - Rotate! Larger load keep the load to avoid feedback variations in voltage.
		int NewL1 = digitalRead(L4);			// Read the value of the outputs in shifted order
		int NewL2 = digitalRead(L1);
		int NewL3 = digitalRead(L2);
		int NewL4 = digitalRead(L3);

		digitalWrite(L1, NewL1);				//Light loads in that shifted order to rotate the loads
		digitalWrite(L2, NewL2);
		digitalWrite(L3, NewL3);
		digitalWrite(L4, NewL4);   
	}
	else {    								//Light the new loads
      switch (loads) {
		  case 0:    
			  digitalWrite(L1, LOW);
			  digitalWrite(L2, LOW);
			  digitalWrite(L3, LOW);
			  digitalWrite(L4, LOW);
			  break;
		  case 1:    
			  digitalWrite(L1, HIGH);
			  digitalWrite(L2, LOW);
			  digitalWrite(L3, LOW);
			  digitalWrite(L4, LOW);
			  break;
		  case 2:    
			  digitalWrite(L1, HIGH);
			  digitalWrite(L2, HIGH);
			  digitalWrite(L3, LOW);
			  digitalWrite(L4, LOW);
			  break;
		  case 3:    
			  digitalWrite(L1, LOW);
			  digitalWrite(L2, HIGH);
			  digitalWrite(L3, HIGH);
			  digitalWrite(L4, HIGH);
			  break;
		  case 4:    
			  digitalWrite(L1, HIGH);
			  digitalWrite(L2, HIGH);
			  digitalWrite(L3, HIGH);
			  digitalWrite(L4, HIGH);
			  break;
		  default:
			  digitalWrite(L1, LOW);
			  digitalWrite(L2, LOW);
			  digitalWrite(L3, LOW);
			  digitalWrite(L4, LOW);
			  break;	
		}
    LastLoad = loads;    
  }
}


void beep(){
  //Do nothng for now
  // tone(Buzz, 1000); // Send 1KHz sound signal...
  // delay(100);        
  // noTone(Buzz);     // Stop sound...
  // delay(100);         
}

//Checks the cells for balance between cells 
int check_balance(){
	int balance_ok = 1;
	float max_voltage = 1.0;
	float min_voltage = 5.0;
  
	sample_cells();           //Get fresh voltages
    
  //check balance
	for (int i=0; i <= (CellCount - 1); i++){          //Loop over cells
		max_voltage = max(Cells[i], max_voltage);        //Find max voltage
		min_voltage = min(Cells[i], min_voltage);        //Find min voltage 
	} 
  
	if ((max_voltage - min_voltage) > max_inbalance ) {
		balance_ok = 0;
	}

	Serial.println(" Misc Check Cell Balance: Min: " + String(min_voltage) + " Max: " + String(max_voltage) );
  if (balance_ok){
    Serial.println(" Misc Cell balance OK");
  }
  
	return balance_ok ;
}


float find_minvoltage(){
  //Checks the cells for least charge
   
  float min_voltage = 10;  //Start with unreasnonable voltage

  sample_cells();           //Get fresh voltages
  
  //check minimum voltagesges
  for (int i=0; i <= (CellCount - 1); i++){
    min_voltage = min(Cells[i], min_voltage);  //Returns the minimum of a cell or least value
    } 
    
	if (debug){Serial.println("Min Voltage: ");}
	if (debug){Serial.println(String(min_voltage));}
    
  return min_voltage ;
}

void stopSketch(void)           //No way back!
    {
    digitalWrite(L1, LOW); //Släck alla loads
    digitalWrite(L2, LOW);
    digitalWrite(L3, LOW);
    digitalWrite(L4, LOW);

    Serial.println("Oper StopSketch");
    delay(1000);
    noInterrupts();
    while(1) {delay(1000);}
    }

int CountCells(void){			//Count and check the cells
	CellCount = 0;			//Local scope
	sample_cells();
	for (int i=0; i <= 3; i++){
	  if (Cells[i] > 3.0 ) {++CellCount;}  //Räkna alla celler med mer än 3 volt
	}    
	return CellCount;
}	
	
	

// Action starts here

void setup() {							
										// Make sure all is off! 			
	pinMode(L1, OUTPUT);
	pinMode(L2, OUTPUT);
	pinMode(L3, OUTPUT);
	pinMode(L4, OUTPUT);
	digitalWrite(L1, LOW); //Släck alla loads
	digitalWrite(L2, LOW);
	digitalWrite(L3, LOW);
	digitalWrite(L4, LOW);

	pinMode(Buzz, OUTPUT);
	analogReference(EXTERNAL);

	Serial.begin(SERIAL_BAUD_RATE); 
	Serial.println(F("\n LiPo Killer v 0.2"));  //Store the static string in flash memory
												
  // Serial.println("Delay plugin ");
	while (CountCells() == 0){
		Serial.println(F(" Oper Waiting for plugin..."));
		//printout();
		delay(2000);
	}
	
	CellCount = CountCells();
  Starttime = millis();
	  
	 Serial.print(F("\nMisc Number of Cells "));
	 Serial.println(CellCount);

	if (find_minvoltage() < min_start_voltage ){           // Starting voltage to low.
		Serial.println(F("\n\n Oper Start voltage too low. Program terminated"));
		Serial.println( find_minvoltage() );
		beep();
		stopSketch();
	}

	if (check_balance() == 0){                            //Starting cell balance not OK.
		Serial.println(F("\n\n Oper Starting cell balance to large. Program terminated"));
		printout();
		beep();
		stopSketch();
	}

	  //Print headline
	  Serial.println(F("\n\n\n\n"));
	  Serial.println(" Status\tTime\tCell1\tCell2\tCell3\tCell4\tCurrent");
}

void loop() {
  // put your main code here, to run repeatedly:

//  load = 0;
  float voltage;

  sample_cells();

  if (millis() >= (LastPrint + PrintInterval)){
		printout();
		LastPrint = millis();
  }
 
  voltage = find_minvoltage();

	//checka obalans?

  //find the loads
  if (voltage <= 3.70){load = 0;}
  if (voltage > 3.720){load = 1;}
  if (voltage > 3.780){load = 2;}
  if (voltage > 3.820){load = 3;}
  if (voltage > 4.000){load = 4;} 
  
//  load = 1;   //Test only
  apply_load(load);

  if (load == 0) {
    Serial.println(F("\n\n Oper Finihed discharge. Program terminated"));
    beep();
    stopSketch();
  }
    
  delay(200);
}









