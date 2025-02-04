/*
 ==============================================================================
 PluginEditor.h
 Author: Thomas Deppisch & Simon Beck
 
 Copyright (c) 2019 - Austrian Audio GmbH
 www.austrian.audio
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

//#define AA_DO_DEBUG_PATH


#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "../resources/lookAndFeel/AA_LaF.h"
#include "../resources/customComponents/TitleBar.h"
#include "../resources/customComponents/SimpleLabel.h"
#include "../resources/customComponents/MuteSoloButton.h"
#include "../resources/customComponents/ReverseSlider.h"
#include "../resources/customComponents/DirSlider.h"
#include "../resources/customComponents/PolarPatternVisualizer.h"
#include "../resources/customComponents/DirectivityEQ.h"
#include "../resources/customComponents/AlertOverlay.h"
#include "../resources/customComponents/EndlessSlider.h"

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

//==============================================================================
/**
*/
class PolarDesignerAudioProcessorEditor  : public AudioProcessorEditor, private Button::Listener,
                                     private ComboBox::Listener, private Slider::Listener, private Timer
{
public:
    PolarDesignerAudioProcessorEditor (PolarDesignerAudioProcessor&, AudioProcessorValueTreeState&);
    ~PolarDesignerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonStateChanged(Button* button) override;
    void buttonClicked (Button* button) override;
    void comboBoxChanged (ComboBox* cb) override;
    void sliderValueChanged (Slider* slider) override;
    
    void onAlOverlayErrorOkay();
    void onAlOverlayApplyPattern();
    void onAlOverlayCancelRecord();
    void onAlOverlayMaxSigToDist();
    void setEqMode();
    float getABButtonAlphaFromLayerState(int layerState);
    // Helper method to calculate flex on the base of bandlimitPathComponents
    std::vector<float> getBandLimitWidthVector(float sizeDirectionalEQ, float offsetPolarVisualizer);
    
    void incrementTrim(int nBands);
    void decrementTrim(int nBands);

    int getControlParameterIndex (Component& control) override;
        
private:
    static const int EDITOR_WIDTH = 990;
    static const int EDITOR_HEIGHT = 630;
    String presetFilename;
    String errorMessage;
        
    const int maxNumberBands = 5;
    int nActiveBands;
    int syncChannelIdx;
    int oldAbLayerState;
    
    bool loadingFile;
    bool recordingDisturber;
    
    Colour eqColours[5];
 
    AALogo logoAA;
    TitleBarAAText titleAA;
    TitleBarPDText titlePD;
    TitleLine titleLine;

    Footer footer;
    LaF globalLaF;
//    LaF2 comboBoxLaF;       // only for ComboBox
    
    PolarDesignerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    TooltipWindow tooltipWindow;

    // Groups
    GroupComponent grpEq, grpPreset, grpDstC, grpProxComp, grpBands, grpSync;
    // Sliders
    ReverseSlider slBandGain[5], slCrossoverPosition[4], slProximity;
    DirSlider slDir[5];
    
    // a slider to use to 'trim' the EQ's
    EndlessSlider trimSlider;
    
    // Solo Buttons
    MuteSoloButton msbSolo[5], msbMute[5];
    // Text Buttons
    TextButton tbLoadFile, tbSaveFile, tbRecordDisturber, tbRecordSignal, tbZeroDelay, tbAbButton[2];
    // ToggleButtons
    ToggleButton tbEq[3], tbAllowBackwardsPattern;
    // Combox Boxes
    ComboBox cbSetNrBands, cbSyncChannel;
    TextButton tbSetNrBands[5];
    TextButton tbSyncChannel[5];
            
    // Pointers for value tree state
    std::unique_ptr<ReverseSlider::SliderAttachment> slBandGainAtt[5], slCrossoverAtt[4], slProximityAtt;
    std::unique_ptr<SliderAttachment> slDirAtt[5];
    std::unique_ptr<ButtonAttachment> msbSoloAtt[5], msbMuteAtt[5], tbAllowBackwardsPatternAtt, tbZeroDelayAtt;
    std::unique_ptr<ComboBoxAttachment> cbSetNrBandsAtt, cbSyncChannelAtt;
    
    DirectivityEQ directivityEqualiser;
    PolarPatternVisualizer polarPatternVisualizers[5];
    AlertOverlay alOverlayError;
    AlertOverlay alOverlayDisturber;
    AlertOverlay alOverlaySignal;

    Path sideBorderPath;
    
#ifdef AA_DO_DEBUG_PATH
    Path debugPath;
#endif
    
    //==========================================================================
    void nActiveBandsChanged();
    void loadFile();
    void saveFile();
    void timerCallback() override;
    bool getSoloActive();
    void disableMainArea();
    void setSideAreaEnabled(bool set);
    void disableOverlay();
    void zeroDelayModeChange();
    
    OpenGLContext openGLContext;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessorEditor)
};
