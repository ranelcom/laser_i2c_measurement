#include "Logger.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdint.h>

#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <cmath>
#include <exception>
#include <chrono>
#include <ctime>
#include <cstring>
#include <iomanip>

#define ADDRESS 0x66

#define SUCCESS 1
#define FAIL -1

#define GPIO_RELAY 			7
#define GPIO_LED_ROJO		0
#define GPIO_LED_VERDE		2
#define GPIO_LED_AMARILLO	3

#define ESTADO_INICIO	0
#define ESTADO_MEDICION_ESTABLE 1
#define ESTADO_CAMBIO_MEDICION 2


int get_config_parameters( std::string);
float parse_command(std::string);
void init_i2c(void);
void send_i2c_command(unsigned char * , unsigned char );
void read_i2c_command(unsigned char * buffer, unsigned char length);
void init_laser(void);
int commando_distancia(float &);
int init_gpio(void);
int WriteCurDist(float); 
	
uint16_t histeresis, n_muestras, ton_relay;

int file_i2c;
int i2cbus;

int get_config_parameters( std::string nom_file) 
{
	try
	{
		//Definimos un archivo 'config'
		//std::ifstream cFile ("config");
		std::ifstream cFile (nom_file);
		//Abrimos el archivo
		if (cFile.is_open()) 
		{
			std::string line;
			while(getline(cFile, line)) 
			{
				line.erase(std::remove_if(line.begin(), line.end(), isspace),line.end());
				if(line[0] == '#' || line.empty())
					continue;
				uint16_t delimiterPos = line.find("=");
				std::string name = line.substr(0, delimiterPos);
				uint16_t value = stoi(line.substr(delimiterPos + 1));
				if( name.compare("histeresis") == 0 ) 
				{
					histeresis = value;
				}
				if( name.compare("n_muestras") == 0 ) 
				{
					n_muestras = value;
				}   
				if( name.compare("ton_relay") == 0 ) 
				{
					ton_relay = value;
				}
			}
		} else 
		{
			//std::cerr << "No puedo abrir archivo config para lectura de configuracion.\n";
			return FAIL;
		}
		return SUCCESS;	
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error get_config_parameters:\n" << e.what() << std::endl;	
		return FAIL;
	}
}

float parse_command(std::string command) 
{

	try 
	{
		uint16_t posicion_inicio_distancia = command.find(":");
		std::string str_distancia = command.substr(posicion_inicio_distancia+1);
		//std::cout << str_distancia << "\n";
		float distancia = stof(str_distancia);
		distancia = distancia * 100.0;
		//std::cout << posicion_inicio_distancia << " " << distancia << " cms\n";
		return distancia;
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error parse_command:\n" << e.what() << std::endl;	
		return 0.0;
	}
}

void init_i2c()
{
	try 
	{
		file_i2c = open("/dev/i2c-1", O_RDWR);

		if (file_i2c < 0) 
		{
			printf("Error al abrir bus I2C\r\n");
			return ;
		}
	
		if( ioctl( file_i2c, I2C_SLAVE, ADDRESS ) < 0 ) 
		{
			printf("Error al abrir direccion I2C\r\n");
			return ;
		}
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error init_i2c:\n" << e.what() << std::endl;	
	}
}

void send_i2c_command(unsigned char * buffer, unsigned char length ) 
{
	
	try
	{
		if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
		{
			/* ERROR HANDLING: i2c transaction failed */
			printf("Failed to write to the i2c bus.\n");
		}
	
		// Retardo necesario para esperar la respuesta. 100us estimado
		usleep(10000);
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error send_i2c_command\n" << e.what() << std::endl;	
	}
}

void read_i2c_command(unsigned char * buffer, unsigned char length) 
{
	
	try 
	{
		if (read(file_i2c, buffer, length) != length)		//read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
		{
			//ERROR HANDLING: i2c transaction failed
			printf("Failed to read from the i2c bus.\n");
			return ;
		}
		return ;
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error read_i2c_command:\n" << e.what() << std::endl;	
	}
}

void init_laser(void) 
{
	unsigned char buffer[60] = {0};
	int length;
	
	try
	{
		printf("Leyendo parametros\r\n");
		buffer[0]='?';buffer[1]='P';buffer[2]='\r';buffer[3]='\n';length = 4;			//<<< Number of bytes to write
		send_i2c_command(buffer, length);
		read_i2c_command(buffer, 32);
		printf("Data read: %s\n", buffer);

		sleep(0.1);	
	
		buffer[0]='?';buffer[1]='L';buffer[2]='M';buffer[3]='\r';buffer[4]='\n';length = 5;			//<<< Number of bytes to write
		send_i2c_command(buffer, length);
		read_i2c_command(buffer, 32);
		printf("Data read: %s\n", buffer);
		sleep(0.1);
	
		buffer[0]='?';buffer[1]='L';buffer[2]='F';buffer[3]='\r';buffer[4]='\n';length = 5;			//<<< Number of bytes to write
		send_i2c_command(buffer, length);
		read_i2c_command(buffer, 32);
		printf("Data read: %s\n", buffer);
		sleep(0.1);
	
		buffer[0]='?';buffer[1]='L';buffer[2]='T';buffer[3]='\r';buffer[4]='\n';length = 5;			//<<< Number of bytes to write
		send_i2c_command(buffer, length);
		read_i2c_command(buffer, 32);
		printf("Data read: %s\n", buffer);
		sleep(0.1);
	
		buffer[0]='?';buffer[1]='L';buffer[2]='N';buffer[3]='\r';buffer[4]='\n';length = 5;			//<<< Number of bytes to write
		send_i2c_command(buffer, length);
		read_i2c_command(buffer, 32);
		printf("Data read: %s\n", buffer);
		sleep(0.1);
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error init laser:\n" << e.what() << std::endl;	
	}
}

int commando_distancia(float &distancia) 
{
	unsigned char buffer[60] = {0};
	int length;
	
	try 
	{
		buffer[0]='?';buffer[1]='L';buffer[2]='D';buffer[3]='F';;buffer[4]='\r';buffer[5]='\n';length = 6;			//<<< Number of bytes to write
		send_i2c_command(buffer, length);
		read_i2c_command(buffer, 32);
	
		if( buffer[0] == 'l') 
		{
			std::string sbuffer(reinterpret_cast<char*>(buffer));
			distancia = parse_command(sbuffer);
			return SUCCESS;
		} else 
		{
			distancia = 0.0;
			return FAIL;
		}
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error comando distancia:\n" << e.what() << std::endl;
		distancia = 0.0;
		return FAIL;
	}
}

int init_gpio(void) {
	try 
	{
		wiringPiSetup();
		pinMode(GPIO_RELAY, OUTPUT);
		pinMode(GPIO_LED_AMARILLO, OUTPUT);
		pinMode(GPIO_LED_ROJO, OUTPUT);
		pinMode(GPIO_LED_VERDE, OUTPUT);
		digitalWrite(GPIO_RELAY, 0);
		digitalWrite(GPIO_LED_AMARILLO, 0);
		digitalWrite(GPIO_LED_ROJO, 0);
		digitalWrite(GPIO_LED_VERDE, 0);
		return SUCCESS;
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error init_gpio:\n" << e.what() << std::endl;	
		return FAIL;
	}
}

int WriteCurDist(float curDistance) 
{

	try 
	{
		//std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		//time_t tt = std::chrono::system_clock::to_time_t(now);

		//auto tt_stripped = std::strtok( ctime(&tt), "\n");
		//auto tt_stripped = std::put_time(std::localtime(&tt), "%d/%m/%Y %h:%m:%s")
		auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		auto tt_stripped = std::put_time(localtime(&now), "%d/%m/%Y %H:%M:%S");
		
		std::ofstream outfile("registro_distancia_actual",std::ios::out);
		outfile <<  "["  << tt_stripped << "] " << curDistance <<" cms" << std::endl;
		outfile.close();
		return SUCCESS;
	}
	catch (std::exception& e) 
	{
		std::cerr << "Error WriteCurDist:\n" << e.what() << std::endl;	
		return FAIL;
	}
}

int main() 
{
	float distancia_medida, ultima_distancia, distancia_estable;
	int estado;
	int contador_distancias;
	
	Logger rpi_log;
		
	init_i2c();
	init_gpio();
	//Envia unos pocos comandos "basura" para inicializar el laser
	init_laser();
	
	rpi_log.open();
	rpi_log.write_log("Power Up!");
	rpi_log.close();
	
	if ( get_config_parameters("config") != SUCCESS) 
	{
		std::cerr << "No se puede abrir archivo config para lectura de configuracion.\n";
	}
	
	printf("Iniciando ciclo lectura distancia laser\r\n");
	estado = ESTADO_INICIO;
	digitalWrite(GPIO_LED_VERDE, 1);
	if( commando_distancia(distancia_medida) != SUCCESS ) 
	{
		distancia_medida = 0;
	}
	digitalWrite(GPIO_LED_VERDE, 0);
	ultima_distancia = distancia_medida;
	contador_distancias = 0;
	while(1) 
	{
		digitalWrite(GPIO_LED_VERDE, 1);
		ultima_distancia = distancia_medida;
		//commando_distancia(distancia_medida);
		if( commando_distancia(distancia_medida) != SUCCESS ) 
		{
			distancia_medida = 0;
		}
		digitalWrite(GPIO_LED_VERDE, 0);
		
		switch(estado) {
			case ESTADO_INICIO:
				if( std::abs(distancia_medida - ultima_distancia) < histeresis) {
					contador_distancias++;
					if(contador_distancias > n_muestras) 
					{
						distancia_estable = distancia_medida;
						contador_distancias = 0;
						std::string buff;
						buff = "Medicion Estable: " + std::to_string(distancia_estable) + " cms!";
						rpi_log.open();
						//rpi_log.write_log("Medicion Estable");
						rpi_log.write_log(buff);
						rpi_log.close();
						estado = ESTADO_MEDICION_ESTABLE;
					}
				} 
				else 
				{
					contador_distancias = 0;
					estado = ESTADO_INICIO;
				}
				
				break;
			case ESTADO_MEDICION_ESTABLE:
				std::cout << std::abs(distancia_medida - distancia_estable) << "\n";
				if( std::abs(distancia_medida - distancia_estable) > histeresis) 
				{
					digitalWrite(GPIO_LED_ROJO, 1);
					contador_distancias++;
					if( contador_distancias > n_muestras) 
					{
						contador_distancias = 0;
						std::string buff;
						buff = "Cambio de distancia: " + std::to_string(distancia_medida) + " cms!";
						rpi_log.open();
						rpi_log.write_log(buff);
						rpi_log.close();
						digitalWrite(GPIO_LED_ROJO, 0);
						estado = ESTADO_CAMBIO_MEDICION;
						
					}
				} 
				else 
				{
					digitalWrite(GPIO_LED_ROJO, 0);
					contador_distancias = 0;
					estado = ESTADO_MEDICION_ESTABLE;
				}
				break;
				
			case ESTADO_CAMBIO_MEDICION:
				digitalWrite(GPIO_LED_AMARILLO, 1);
				digitalWrite(GPIO_RELAY, 1);
				sleep(ton_relay);
				digitalWrite(GPIO_LED_AMARILLO, 0);
				digitalWrite(GPIO_RELAY, 0);
				estado = ESTADO_INICIO;
				break;
				
		}
		std::cout << "Distancia actual: " << distancia_medida << "cms "; 
		std::cout << "Ultima medicion: " << ultima_distancia << "cms "; 
		std::cout << "Distancia estable: " << distancia_estable << "cms ";
		std::cout << "Contador Distancias: " << contador_distancias << " ";
		std::cout << "Estado: " << estado << "\n";
		WriteCurDist(distancia_medida); 
		sleep(1);
	}
	
	return SUCCESS;
}
