void setup()
{
    Serial.begin(9600);
}

void loop()
{
    int sensorValue1 = analogRead(A4);
    int sensorValue2 = analogRead(A5);

    // Serial.print("The X and Y coordinate is:");
    // Serial.print(sensorValue1, DEC);
    // Serial.print(",");
    // Serial.println(sensorValue2, DEC);
    // Serial.println(" ");
    // delay(200);
}
