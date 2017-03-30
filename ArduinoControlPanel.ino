#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>
#include <DHT.h>

#define REQUEST_BUFFER_SIZE 20

const char indexHtml[] PROGMEM  = {
"<!DOCTYPE HTML>" \
"<html>" \
"<head>" \
"<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\"/>"\
"<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css\"/>"\
"<title>Arduino Control Panel</title>"\
"</head>"\
"<body>"\
"<div class=\"container\">"\
"<h1>Arduino Control Panel</h1>"\
"<h3>RELAY</h3>"\
"<div class=\"row\">"\
"<div class=\"col-md-12\">"\
// copy this line if you want control more LEDs
"<button class=\"btn btn-default btn-led\" data-led=\"9\">LED 9 <span>OFF</span></button>"\
"</div>"\
"</div>"\
"<h3>SENSORS</h3>"\

"<div class=\"row\">"\
"<div class=\"col-md-4\">"\
"<div class=\"panel panel-default\">"\
"<div class=\"panel-heading\">Room 1</div>"\
"<div class=\"panel-body text-center\">"\
"<div id=\"temp-gauge\"></div>"\
"</div>"\
"</div>"\
"</div>"\
"</div>"\

"</div>"\
"<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js\"></script>"\
"<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\"></script>"\
"<script src=\"http://mirobozik.com/Media/raphael-2.1.4.min.js\"></script>"\
"<script src=\"http://mirobozik.com/Media/justgage.js\"></script>"\
"<script src=\"http://mirobozik.com/Media/acp.js\"></script>"\
"</body>"\
"</html>"
};

byte mac[] = {
	0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetServer server(80);
boolean ledStatus;

char httpRequest[REQUEST_BUFFER_SIZE] = {0};
char requestIndex = 0;
DHT dht(2, DHT11);

void setup()
{
	pinMode(9, OUTPUT);
	dht.begin();
	
	// Open serial communications and wait for port to open:
	Serial.begin(9600);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	// start the Ethernet connection and the server:
	Ethernet.begin(mac);
	server.begin();
	Serial.print("server is at ");
	Serial.println(Ethernet.localIP());
}

void loop()
{
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) 
	{
		Serial.println("new client");
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) 
		{
			if (client.available()) 
			{
				char c = client.read();
				if (requestIndex < (REQUEST_BUFFER_SIZE - 1))
				{
					httpRequest[requestIndex] = c;
					requestIndex++;
				}
				
				//Serial.write(c);
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) 
				{
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Connection: close");
					
					if(strContains(httpRequest, "POST /LED9"))
					{
						client.println();
						setLedState(client);
					}
					if(strContains(httpRequest, "GET / "))
					{
						client.println("Content-Type: text/html");
						client.println();

						int len = strlen_P(indexHtml);
						for (int k = 0; k < len; k++)
						{
							char c = pgm_read_byte_near(indexHtml + k);
							Serial.print(c);
							client.print(c);
						}
					}
					if(strContains(httpRequest, "GET /temp "))
					{						
						client.println();
						float temp = dht.readTemperature();
						client.print(temp);
					}
					
					requestIndex = 0;
					strClear(httpRequest, REQUEST_BUFFER_SIZE);
					
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
					} else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
		Serial.println("client disconnected");
	}
}

void setLedState(EthernetClient cl)
{
	ledStatus = !ledStatus;
	if(ledStatus){
		digitalWrite(9, HIGH);
		cl.print("ON");
		} else {
		digitalWrite(9, LOW);
		cl.print("OFF");
	}
}

void strClear(char *str, char length)
{
	for (int i = 0; i < length; i++) {
		str[i] = 0;
	}
}

char strContains(char *str, char *sfind)
{
	char found = 0;
	char index = 0;
	char len;

	len = strlen(str);
	
	if (strlen(sfind) > len) {
		return 0;
	}
	while (index < len) {
		if (str[index] == sfind[found]) {
			found++;
			if (strlen(sfind) == found) {
				return 1;
			}
		}
		else {
			found = 0;
		}
		index++;
	}

	return 0;
}
