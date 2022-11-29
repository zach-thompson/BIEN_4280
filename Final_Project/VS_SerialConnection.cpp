// BioCFinal.cpp : This file contains the 'main' function. Program execution begins and ends there

// <code>
#include <iostream>
#include <speechapi_cxx.h>
#include<windows.h>
#include<stdio.h>
#include <string.h>
#include <strsafe.h>
#include <tchar.h>
#include "AudioFile.h"
#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define BSIZE           200
#define READ_TIMEOUT    1000 //
#include <vector>
#include <sstream>
#include <iterator>

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
using namespace Microsoft::CognitiveServices::Speech::Audio;
using namespace Microsoft::CognitiveServices::Speech::Translation;

HANDLE hComm;
BOOL Status, RStatus;
char  ReadData[44100] = { 0 };                        // Temperory Character
char ReadData2[100] = { 0 };
const char* RData;
DWORD dwEventMask;                     // Event mask to trigger
char  SerialBuffer[64] = { 0 };               // Buffer Containing Rxed Data
DWORD NoBytesRecieved;                 // Bytes read by ReadFile()
DWORD BytesWritten = 0;          // No of bytes written to the port
DWORD NoBytesRead;     // Bytes read by ReadFile()
DCB dcbSerialParams = { 0 };                         // Initializing DCB structure
COMMTIMEOUTS timeouts = { 0 };
//int i = 0;


void recognizeSpeech()
{
    // Creates an instance of a speech config with specified subscription key and service region.
    // Replace with your own subscription key and service region (e.g., "westus").
    hComm = CreateFileA("COM7",                //port name
        GENERIC_READ | GENERIC_WRITE, //Read/Write
        FILE_SHARE_READ,                     // No Sharing
        NULL,                         // No Security
        OPEN_EXISTING,// Open existing port only
        FILE_ATTRIBUTE_NORMAL,            // Non Overlapped I/O
        NULL);        // Null for Comm Devices

    if (hComm == INVALID_HANDLE_VALUE) {
        printf("Error in opening serial port");
        //return GetLastError();
    }
    else {
        printf("opening serial port successful");
    }

    // Write data to the file

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    Status = GetCommState(hComm, &dcbSerialParams);      //retreives  the current settings

    if (Status == FALSE) {
        printf("\n    Error! in GetCommState()");
    }

    dcbSerialParams.BaudRate = CBR_9600;      // Setting BaudRate = 9600
    dcbSerialParams.ByteSize = 8;             // Setting ByteSize = 8
    dcbSerialParams.StopBits = 1;    // Setting StopBits = 1
    dcbSerialParams.Parity = NOPARITY;        // Setting Parity = None 

    Status = SetCommState(hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

    if (Status == FALSE)
    {
        printf("\n    Error! in Setting DCB Structure");
    }
    else //If Successfull display the contents of the DCB Structure
    {
        printf("\n\n    Setting DCB Structure Successfull\n");
        printf("\n       Baudrate = %d", dcbSerialParams.BaudRate);
        printf("\n       ByteSize = %d", dcbSerialParams.ByteSize);
        printf("\n       StopBits = %d", dcbSerialParams.StopBits);
        printf("\n       Parity   = %d\n", dcbSerialParams.Parity);
    }

    //Setting Timeouts

    timeouts.ReadIntervalTimeout = 5;
    timeouts.ReadTotalTimeoutConstant = 5;
    timeouts.ReadTotalTimeoutMultiplier = 5;
    timeouts.WriteTotalTimeoutConstant = 5;
    timeouts.WriteTotalTimeoutMultiplier = 5;


    // 1. Create an AudioBuffer 
    // (BTW, AudioBuffer is just a vector of vectors)

    AudioFile<double> audioFile;
    // 1. Set a file path to an audio file on your machine
    const std::string inputFilePath = std::string("C:/Users/zacht/Downloads/test-audio1.wav");

    //---------------------------------------------------------------
    // 2. Create an AudioFile object and load the audio file


    bool loadedOK = audioFile.load(inputFilePath);
    //audioFile.load("C:/Users/kattx/OneDrive- Marquette University/Fall21/BioCLab/FinalProject/examples_test-audio.wav");
    AudioFile<double>::AudioBuffer buffer;
    //AudioFile<char>::AudioBuffer buffer[24000];
    int sampleRate = audioFile.getSampleRate();
    int bitDepth = audioFile.getBitDepth();

    int numSamples = audioFile.getNumSamplesPerChannel();
    double lengthInSeconds = audioFile.getLengthInSeconds();

    int numChannels = audioFile.getNumChannels();
    bool isMono = audioFile.isMono();
    bool isStereo = audioFile.isStereo();

    // or, just use this quick shortcut to print a summary to the console
    audioFile.printSummary();

    // 2. Set to (e.g.) two channels
    buffer.resize(1);

    int sizer = 20000;
    // 3. Set number of samples per channel
    buffer[0].resize(10000);
    //buffer[1].resize(44100);

    // 4. do something here to fill the buffer with samples, e.g.
    cout << "\nSay something...\n";

    EscapeCommFunction(hComm, CLRDTR);
    EscapeCommFunction(hComm, SETDTR);

    //for (int i = 0; i < 100; i++) {
    RStatus = ReadFile(hComm,// Handle to the Serialport
        ReadData,            // Data to be read to the port
        sizer,// No of bytes to read into the port
        &NoBytesRead,  // No of bytes written read to the port
        NULL);

    cout << "Record Over";
    //Get Actual length of received data
    //printf_s("\nNumber of bytes received = %d\n\n", NoBytesRead);
    //print receive data on console
    //printf("ReadData:...%d %d %d %d", ReadData[0], ReadData[1], ReadData[2], ReadData[3]);

    //uint16_t number = (ReadData[0] << 8) | ReadData[1];
    //printf("\n\nReadData[0] and [1] combined Which was...%d", number);

    int16_t buff[10000] = { 0 };
    int j = 0;
    for (int i = 0; i < sizer; i = i + 2) {

        buff[j] = (ReadData[i + 1] << 8) | ReadData[i];
        //printf("\n\nReadData[0] and [1] combined Which was...%d", buff[j]);
        j++;
    }
    printf("\nbuff size = %d \n", j);
    //combine back into two 16
    //put into array
    float cutoff = 200;
    double RC = 1.0 / (cutoff * 2 * 3.14);
    double dt = 1.0 / (16000);
    double alpha = dt / (RC + dt);
    buffer[0][0] = ((float)buff[0] / 10000.0);

    for (int i = 1; i < 10000; i++)
    {

        //buffer[0][i] = ((float)buff[i] / 10000.0);
        buffer[0][i] = buffer[0][i - 1] + (alpha * (((float)buff[i] / 10000.0) - buffer[0][i - 1]));
        //cout << "AudioBuffer data: " << buffer[0][i]  << "\n";
    }
    //buffer[0][i];
// }


// then...
   //addStringtoFileData(ReadData);
   //audioFile.load(ReadData);

   //bool writeDataToFile(std::vector<uint8_t>&ReadData, std::string inputFilePath);

   //char* s = (char*)&ReadData;
   //string s;
   /* char newString[44100][44100];
   //cout << s[1];
    //fgets(ReadData, sizeof ReadData, stdin);

   int j = 0; int ctr = 0;
    for (int x = 0; x <= (strlen(ReadData)); x++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if (ReadData[x] == ' ' || ReadData[x] == '\0' || ReadData[x] == '\n')
        {
            newString[ctr][j] = '\0';
            ctr++;  //for next word
            j = 0;    //for next word, init index to 0
        }
        else
        {
            newString[ctr][j] = ReadData[x];
            j++;
        }
    }

  /* char output[44100];
  for (int x = 0; x < 44100; x++) {

       _itoa_s(ReadData[x], output, 2);

   }

   //char s[44100];
   */
   // cout << "\n" << buffer[0][0] << ReadData;
   /* for (int i = 0; i < 44099; i++)
   {

          cout << "\n" << buffer[0][i] << ReadData;
           //cout << buffer[0][i];
           //<< ReadData[i];
   };
    */

    //bool AudioFile<audioFile>::saveToWaveFile(std::string ReadData)
       //addStringtoFileData(ReadData)
       //std::istringstream iss(ReadData[44099]);
        //std::istream_iterator<double> it(iss), end;
       //AudioFile<double>::AudioBuffer v(it, end);

       //cout << v.size() << endl;

      // std::cout << v;
       // 2. Set to (e.g.) two channels
      // v.resize(1);

       // 3. Set number of samples per channel
       //v[0].resize(44100);
      // v[1].resize(10000);


            // 5. Put into the AudioFile object
        //audioFile.setAudioBuffer(AudioFile<stringbuf>::AudioBuffer& ReadData);
    bool ok = audioFile.setAudioBuffer(buffer);

    // Set both the number of channels and number of samples per channel
   //audioFile.setAudioBufferSize(numChannels, numSamples);

    // Set the number of samples per channel
    //audioFile.setNumSamplesPerChannel(numSamples);

    // Set the number of channels
    //audioFile.setNumChannels(numChannels);

    audioFile.setBitDepth(16);
    audioFile.setSampleRate(16000);

    audioFile.save("C:/Users/kattx/Downloads/test-audio1.wav", AudioFileFormat::Wave);

    auto Config = SpeechTranslationConfig::FromSubscription("b4634e5633224ef0af629a51752f87ee", "centralus");

    Config->SetSpeechRecognitionLanguage("en-US");
    Config->SetSpeechRecognitionLanguage("es-MX");

    auto audioInput = AudioConfig::FromWavFileInput("C:/Users/kattx/Downloads/test-audio1.wav");
    auto recognizer = SpeechRecognizer::FromConfig(Config, audioInput);

    // promise for synchronization of recognition end.
    promise<void> recognitionEnd;

    std::string referenceText = "";
    // Creates a speech recognizer.

    // Subscribes to events.
    recognizer->Recognizing.Connect([](const SpeechRecognitionEventArgs& e)
        {
            cout << "Recognizing:" << e.Result->Text << std::endl;
            if ((e.Result->Text == "One") || (e.Result->Text == "Uno") || (e.Result->Text == "uno") || (e.Result->Text == "one") || (e.Result->Text == "One.") || (e.Result->Text == "Uno.") || (e.Result->Text == "uno.") || (e.Result->Text == "one.") || (e.Result->Text == "1.") || (e.Result->Text == "1")) {
                SerialBuffer[0] = 1;
            }
            else if ((e.Result->Text == "Two") || (e.Result->Text == "Dos") || (e.Result->Text == "dos") || (e.Result->Text == "two") || (e.Result->Text == "Two.") || (e.Result->Text == "Dos.") || (e.Result->Text == "dos.") || (e.Result->Text == "two.") || (e.Result->Text == "2.") || (e.Result->Text == "2")) {
                SerialBuffer[0] = 2;
            }
            else if ((e.Result->Text == "Three") || (e.Result->Text == "Tres") || (e.Result->Text == "tres") || (e.Result->Text == "three") || (e.Result->Text == "Three.") || (e.Result->Text == "Tres.") || (e.Result->Text == "tres.") || (e.Result->Text == "three.") || (e.Result->Text == "3.") || (e.Result->Text == "3")) {
                SerialBuffer[0] = 3;
            }
            else if ((e.Result->Text == "Four") || (e.Result->Text == "Cuatro") || (e.Result->Text == "cuatro") || (e.Result->Text == "four") || (e.Result->Text == "Four.") || (e.Result->Text == "Cuatro.") || (e.Result->Text == "cuatro.") || (e.Result->Text == "four.") || (e.Result->Text == "4.") || (e.Result->Text == "4")) {
                SerialBuffer[0] = 4;
            }
            else if ((e.Result->Text == "Five") || (e.Result->Text == "Cinco") || (e.Result->Text == "cinco") || (e.Result->Text == "five") || (e.Result->Text == "Five.") || (e.Result->Text == "Cinco.") || (e.Result->Text == "cinco.") || (e.Result->Text == "five.") || (e.Result->Text == "5.") || (e.Result->Text == "5")) {
                SerialBuffer[0] = 5;
            }
            else {
                SerialBuffer[0] = 0;
            }
        });




    // create pronunciation assessment config, set grading system, granularity and if enable miscue based on your requirement.
    auto pronunciationConfig = PronunciationAssessmentConfig::Create(referenceText,
        PronunciationAssessmentGradingSystem::HundredMark,
        PronunciationAssessmentGranularity::Phoneme, true);


    pronunciationConfig->SetReferenceText(referenceText);
    //auto recognizer = SpeechRecognizer::FromConfig(Config);


    // Starts speech recognition, and returns after a single utterance is recognized. The end of a
    // single utterance is determined by listening for silence at the end or until a maximum of 15
    // seconds of audio is processed.  The task returns the recognition text as result. 
    // Note: Since RecognizeOnceAsync() returns only a single utterance, it is suitable only for single
    // shot recognition like command or query. 
    // For long-running multi-utterance recognition, use StartContinuousRecognitionAsync() instead.

    pronunciationConfig->ApplyTo(recognizer);

    // auto result = recognizer->RecognizeOnceAsync().get();
     // Checks result.
    recognizer->Recognized.Connect([](const SpeechRecognitionEventArgs& e) {
        if (e.Result->Reason == ResultReason::RecognizedSpeech)
        {


            std::string referenceText = e.Result->Text;

            // Checks result.

            cout << "RECOGNIZED: Text = " << e.Result->Text << "\n"
                << "  Offset=" << e.Result->Offset() << "\n"
                << "  Duration=" << e.Result->Duration()

                << " \n PRONUNCIATION ASSESSMENT RESULTS:\n";

            auto pronunciationResult = PronunciationAssessmentResult::FromResult(e.Result);

            //cout << " Accuracy score: " << pronunciationResult->AccuracyScore << "\n Pronunciation score: "
              //  << pronunciationResult->PronunciationScore << "\n Completeness score: " << pronunciationResult->CompletenessScore
                //<< "\n FluencyScore: " << pronunciationResult->FluencyScore << endl;

            EscapeCommFunction(hComm, CLRRTS);
            EscapeCommFunction(hComm, SETRTS);
            //Writing data to Serial Port
            Status = WriteFile(hComm,// Handle to the Serialport
                SerialBuffer,            // Data to be written to the port
                sizeof(SerialBuffer),   // No of bytes to write into the port
                &BytesWritten,  // No of bytes written to the port
                NULL);
            if (Status == FALSE)
            {
                printf("\nFail to Write");

            }
            printf("\nNumber of bytes written to the serial port = %d\n\n", BytesWritten);


            EscapeCommFunction(hComm, CLRDTR);
            EscapeCommFunction(hComm, SETDTR);
            RStatus = ReadFile(hComm,// Handle to the Serialport
                ReadData2,            // Data to be written to the port
                11,   // No of bytes to read into the port
                &NoBytesRead,  // No of bytes written read to the port
                NULL);
            //Get Actual length of received data
            //printf_s("\nNumber of bytes received = %d\n\n", NoBytesRead);
            //print receive data on console
            //cout << "\n\n Which was...%f" << ReadData2;
            int16_t buff2[100] = { 0 };
            int k = 0;
            for (int i = 0; i < 2; i = i + 2) {

                buff2[k] = (ReadData2[i + 1] << 8) | ReadData2[i];
                //printf("\n\nReadData[0] and [1] combined Which was...%d", buff[j]);
                k++;
            }
            printf("\n\nReturned Data: %i \n", buff2);
        }
        else if (e.Result->Reason == ResultReason::NoMatch)
        {
            cout << "NOMATCH: Speech could not be recognized." << std::endl;
        }
        });

    recognizer->Canceled.Connect([&recognitionEnd](const SpeechRecognitionCanceledEventArgs& e)
        {
            cout << "CANCELED: Reaqzson=" << (int)e.Reason << std::endl;

            if (e.Reason == CancellationReason::Error)
            {
                cout << "CANCELED: ErrorCode=" << (int)e.ErrorCode << "\n"
                    << "CANCELED: ErrorDetails=" << e.ErrorDetails << "\n"
                    << "CANCELED: Did you update the subscription info?" << std::endl;

                recognitionEnd.set_value(); // Notify to stop recognition.
            }
        });

    recognizer->SessionStopped.Connect([&recognitionEnd](const SessionEventArgs& e)
        {
            cout << "Session stopped.";
            recognitionEnd.set_value(); // Notify to stop recognition.
        });

    // Starts continuous recognition. Uses StopContinuousRecognitionAsync() to stop recognition.
    recognizer->StartContinuousRecognitionAsync().get();

    // Waits for recognition end.
    recognitionEnd.get_future().get();

    // Stops recognition.
    recognizer->StopContinuousRecognitionAsync().get();
    // </SpeechContinuousRecognitionWithFile>
}
/*int wmain()
{
    try
    {
        recognizeSpeech();
    }
    catch (exception e)
    {
        cout << e.what();
    }
    cout << "Please press a key to continue.\n";
    cin.get();
    return 0;
}*/
// </code>
int main() {


    try
    {
        recognizeSpeech();
    }
    catch (exception e)
    {
        cout << e.what();
    }
    cout << "\nPlease press enter to continue.\n";
    cin.get();
    CloseHandle(hComm);//Closing the Serial Port
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
