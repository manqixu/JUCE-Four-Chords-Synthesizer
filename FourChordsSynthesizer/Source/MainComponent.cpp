// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
//
// Author: Maggie Xu (manqixu@stanford.edu)
// Description: Simple JUCE Four Chord Synthesizer

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"

// Number of sine waves
const int NumSine = 3;

// Seven Pitches starting with C4
const float pitch[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};

// Four Chords: C Am F G
const int NumChord = 4;
const float chord[NumChord][NumSine] = {{pitch[0], pitch[2], pitch[4]},{pitch[0],pitch[2],pitch[5]},{pitch[3],pitch[5], pitch[0]},{pitch[1],pitch[4], pitch[6]}}; 

class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private Button::Listener
{
public:
    MainContentComponent() : keyC('C'), keyA('A'), keyF('F'), keyG('G'), onOff (0), samplingRate(0.0)
    {
        for (int i = 0; i < NumSine; i++) {
            // configuring frequency slider and adding it to the main window
            addAndMakeVisible (frequencySlider[i]);
            frequencySlider[i].setRange (50.0, 5000.0);
            frequencySlider[i].setSkewFactorFromMidPoint (500.0);
            frequencySlider[i].setValue(1000); // will also set the default frequency of the sine osc
            frequencySlider[i].addListener (this);
            
            // configuring frequency label box and adding it to the main window
            addAndMakeVisible(frequencyLabel[i]);
            frequencyLabel[i].setText ("Frequency", dontSendNotification);
            frequencyLabel[i].attachToComponent (&frequencySlider[i], true);
            
            // configuring gain slider and adding it to the main window
            addAndMakeVisible (gainSlider[i]);
            gainSlider[i].setRange (0.0, 1.0);
            gainSlider[i].setValue(0.5); // will alsi set the default gain of the sine osc
            gainSlider[i].addListener (this);
            
            // configuring gain label and adding it to the main window
            addAndMakeVisible(gainLabel[i]);
            gainLabel[i].setText ("Gain", dontSendNotification);
            gainLabel[i].attachToComponent (&gainSlider[i], true);
        }
        
        
        // configuring on/off button and adding it to the main window
        addAndMakeVisible(onOffButton);
        onOffButton.addListener(this);
        
        // configuring on/off label and adding it to the main window
        addAndMakeVisible(onOffLabel);
        onOffLabel.setText ("On/Off", dontSendNotification);
        onOffLabel.attachToComponent (&onOffButton, true);
        
        // configuring chord buttons and adding them to the main window
        for (int i = 0; i < NumChord; i++) {
            addAndMakeVisible(chordButton[i]);
            chordButton[i].addListener(this);
        }
        
        // configuring chord button names
        chordButton[0].setButtonText("C");
        chordButton[1].setButtonText("Am");
        chordButton[2].setButtonText("F");
        chordButton[3].setButtonText("G");   
        
        setSize (600, 300);
        nChans = 1;
        setAudioChannels (0, nChans); // no inputs, one output
        
        setWantsKeyboardFocus (true);
        
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    void resized() override
    {
        // placing the UI elements in the main window
        // getWidth has to be used in case the window is resized by the user
        const int sliderLeft = 80;
        const int sliderHeight = 80;
        for (int i = 0; i < NumSine; i++) {      
            frequencySlider[i].setBounds (sliderLeft, 10 + i * sliderHeight, getWidth() - sliderLeft - 20, 20);
            gainSlider[i].setBounds (sliderLeft, 40 + + i * sliderHeight, getWidth() - sliderLeft - 20, 20);
        }
        onOffButton.setBounds (sliderLeft, 70 + (NumSine-1) * sliderHeight, getWidth() - sliderLeft - 20, 20);
        
        for (int i = 0; i < NumChord; i++) {
            chordButton[i].setBounds (sliderLeft + i * 100, 100 + (NumSine - 1) * sliderHeight, 100, 20); 
        }
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (samplingRate > 0.0){
            for(int i = 0; i < NumSine; i++){      
                if (slider == &frequencySlider[i]){
                    sine[i].setFrequency(frequencySlider[i].getValue());
                }
                else if (slider == &gainSlider[i]){
                    gain[i] = gainSlider[i].getValue();
                }
            }
        }
    }
    
    void buttonClicked (Button* button) override
    {
        // turns audio on or off
        if(button == &onOffButton){
            if(onOffButton.getToggleState()){
                onOff = 1;
            } else {
                onOff = 0;
            }
        } else {
            for (int i = 0; i < NumChord; i++) {
                if(button == &chordButton[i]){
                    for (int j = 0; j < NumSine; j++){
                        frequencySlider[j].setValue(chord[i][j]);
                    }
                }
            }
        }
    }
    
    
    // configuring keypress listener
    bool keyPressed (const KeyPress &key) override
    {
        for (int i = 0; i < NumChord; i++) {
            if(key == *keys[i]){
                for (int j = 0; j < NumSine; j++) {
                    frequencySlider[j].setValue(chord[i][j]);
                }
            }
        }
        return true;   
    }
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        samplingRate = sampleRate;
        for (int i = 0; i < NumSine; i++) {
            sine[i].setSamplingRate(sampleRate);
        }
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // getting the audio output buffer to be filled
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        
        // computing one block
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            if(onOff == 1) {
                for (int i = 0; i < NumSine; i++){
                    buffer[sample] += sine[i].tick() * gain[i];
                }
                buffer[sample] = buffer[sample] / NumSine;
            }
            else buffer[sample] = 0.0;
        }
    }
    
    
private:

    // UI Elements
    Slider frequencySlider[NumSine];
    Slider gainSlider[NumSine];
    ToggleButton onOffButton;
    
    TextButton chordButton[NumChord];
    
    // KeyPress Elements
    KeyPress keyC;
    KeyPress keyA;
    KeyPress keyF;
    KeyPress keyG;
    KeyPress* keys[NumChord] = {&keyC, &keyA, &keyF, &keyG};
            
    Label frequencyLabel[NumSine], gainLabel[NumSine], onOffLabel;
    
    Sine sine[NumSine]; // the sine wave oscillators
    
    // Global Variables
    float gain[NumSine] = {0.0};
    int onOff, samplingRate, nChans;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
