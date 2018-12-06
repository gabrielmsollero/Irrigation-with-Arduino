/* The idea of this project is to unify irrigation and weather forecast. The control of humidity is made according to the rain probability:
if the chances are high, the humidity is maintained at a low level so that the water from the rain elevates the humidity to its optimal
level. Of course this is just an idea for now: we didn't obtain actual weather forecast data and didn't do experimental tests.
The forecast data would be obtained from a ESP-01 connected to the internet, from theweatherchannel.com's API for forecasts.
Authors: andrefavotto, gabrielmsollero, vitorbf37. */
//declaração das variáveis
const byte minimo = 20; //valores hipotéticos de mínimo e ideal
const byte ideal = 50; 
const byte rele = 3; //módulos utilizados
const byte sensorchuva = 2; 
const byte sensorumid = A0;
short int prob = -1;//variável na qual será armazenado, posteriormente, o valor da probabilidade de chuva

void setup() {
  pinMode(sensorumid,INPUT); 
  pinMode(sensorchuva,INPUT);
  pinMode(rele,OUTPUT);
  Serial.begin(9600); //debug e obtenção da probabilidade
  Serial.println("Insira a probabilidade de chuva:");
  while(prob == -1) //o laço é encerrado quando existe uma probabilidade de chuva
  prob = ler_probabilidade();
}

int ler_probabilidade()//função que obtém o valor da probabilidade via Serial
{
  int p = -1;
  if(Serial.available())
      p = Serial.parseInt();
  }                                        
  return p; //se for incrementada sem alterações comprometerá o laço
}

int chovendo()//função que identifica a presença de chuva
{
  bool v1=true; //inicializando o valor das três verificações como "true",
  bool v2=true; //pois "false" indica presença de chuva
  bool v3=true;
  for(byte i=0;i<3;i++){
   v1=v2;                      //três verificações, para evitar retornos equivocados
   v2=v3;                      //provocados por respingos, por exemplo
   v3=digitalRead(sensorchuva);
   delay(3000);               
  }//é necessário que o sensor receba água por 9s, o que é muito difícil em um caso que não seja chuva, ou seja, as chances de dar um falso retorno de chuva são muito baixas
  if(v1==false && v2==false && v3==false)
  return 1;//se as três verificações indicarem chuva, retorno verdadeiro
  else
  return 0;
}
//caso uma chuva comece no meio de uma verificação, será identificada na próxima
int ler_umidade()//função que converte o retorno do sensor de umidade para %
{
  return map(analogRead(sensorumid),1023,0,0,100);
}//converte o intervalo do sensor para porcentagem
//o sensor retorna o valor máximo quando a umidade é mínima

void loop() {
  short int umidade = ler_umidade();
  if(chovendo()==1) //único comando executado durante a chuva
  digitalWrite(rele, LOW);
  else //a possibilidade de irrigação só é avaliada se não estiver chovendo
  {
    umidade = ler_umidade(); //a base da tomada de decisão é a umidade atual,
    if(umidade<=minimo){     //que deve ser mantida sempre entre mínimo e ideal, o primeiro if é uma medida de segurança p/ impedir que a umidade esteja abaixo do ponto de murcha permanente
      while(umidade<minimo+5){//elevar a umidade até que esteja acima do mínimo, com uma margem para garantir que a solenoide nao ligue e desligue repetidamente
      if(chovendo()==1)
      {//em caso de chuva durante a irrigação, desligar o relé e sair do laço
        digitalWrite(rele,LOW);
        break;
      }
      umidade = ler_umidade();
      digitalWrite(rele,HIGH);
      }
      }
    if(umidade>=ideal)
    {
    digitalWrite(rele,LOW);//garantia para não irrigar de forma alguma se a umidade estiver acima do ideal
    }
    
    if(umidade>minimo && umidade<ideal)
    { //cálculo da umidade a ser mantida, de acordo com a probabilidade de chuva
      float umidmantida = minimo + ((100-prob)/100.0)*(ideal-minimo);//o resultado está limitado ao intervalo entre mínimo e ideal, e o que varia-o é a probabilidade
      while(umidade<=umidmantida+5) // que, conforme aumenta, aproxima o resultado do mínimo devido ao fator (100-prob)
      {
      if(chovendo()==1)//verificação constante da presença de chuva
      {
        digitalWrite(rele,LOW);
        break;
      }
      umidade = ler_umidade();  
      digitalWrite(rele,HIGH);     
      }
      digitalWrite(rele,LOW);//desligando a solenoide após atingir a umidade calculada, pois nenhum outro if o fará a não ser que supere o valor ideal
    }
  }}
