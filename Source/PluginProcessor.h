/*
 ==============================================================================
 PluginProcessor.h
 Author: Thomas Deppisch
 
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

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <memory> // for unique_ptr
#include <math.h>
#include <fftw3.h>

//==============================================================================
/**
*/
class PolarDesignerAudioProcessor  : public AudioProcessor, public AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    PolarDesignerAudioProcessor();
    ~PolarDesignerAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    void parameterChanged (const String &parameterID, float newValue) override;
    
    //==============================================================================
    Result loadPreset (const File& presetFile);
    Result savePreset (File destination);
    File getLastDir() {return lastDir;}
    void setLastDir(File newLastDir);
    
    void startTracking(bool trackDisturber);
    void stopTracking(int applyOptimalPattern);
    
    int getNBands() {return nBands;}
    float getXoverSliderRangeStart (int sliderNum);
    float getXoverSliderRangeEnd (int sliderNum);
    Atomic<bool> repaintDEQ = true;
    Atomic<bool> nActiveBandsChanged = true;
    Atomic<bool> zeroDelayModeChanged = true;
    Atomic<bool> ffDfEqChanged = true;
    bool getDisturberRecorded() {return disturberRecorded;}
    bool getSignalRecorded() {return signalRecorded;}
    
    // initial xover frequencies for several numbers of bands
    const float INIT_XOVER_FREQS_2B[1] = {1000.0f};
    const float INIT_XOVER_FREQS_3B[2] = {250.0f,3000.0f};
    const float INIT_XOVER_FREQS_4B[3] = {200.0f,1000.0f,5000.0f};
    const float INIT_XOVER_FREQS_5B[4] = {150.0f,600.0f,2600.0f,8000.0f};
    
    // xover ranges for several numbers of bands
    const float XOVER_RANGE_START_2B[1] = {120.0f};
    const float XOVER_RANGE_END_2B[1] = {12000.0f};
    const float XOVER_RANGE_START_3B[2] = {120.0f, 2000.0f};
    const float XOVER_RANGE_END_3B[2] = {1000.0f, 12000.0f};
    const float XOVER_RANGE_START_4B[3] = {120.0f, 900.0f, 4000.0f};
    const float XOVER_RANGE_END_4B[3] = {450.0f, 2500.0f, 12000.0f};
    const float XOVER_RANGE_START_5B[4] = {120.0f, 500.0f, 2200.0f, 7000.0f};
    const float XOVER_RANGE_END_5B[4] = {200.0f, 1100.0f, 4000.0f, 12000.0f};
    
    int getEqState() {return doEq;}
    void setEqState(int idx) {doEq = idx;}
    float hzToZeroToOne(int idx, float hz);
    float hzFromZeroToOne(int idx, float val);
    bool zeroDelayModeActive() {return *zeroDelayMode != 0.0f;}
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolarDesignerAudioProcessor)
    
    int nBands;

    AudioProcessorValueTreeState params;
    static const int N_CH_IN = 2;
    
    // use odd FIR_LEN for even filter order (FIR_LEN = N+1)
    // (lowpass and highpass need even filter order to put a zero at f=0 and f=pi)
    int firLen;
        
    // free field / diffuse field eq
    dsp::Convolution dfEqOmniConv;
    dsp::Convolution dfEqEightConv;
    AudioBuffer<float> dfEqOmniBuffer;
    AudioBuffer<float> dfEqEightBuffer;
    dsp::Convolution ffEqOmniConv;
    dsp::Convolution ffEqEightConv;
    AudioBuffer<float> ffEqOmniBuffer;
    AudioBuffer<float> ffEqEightBuffer;
    
    // proximity compensation filter
    dsp::IIR::Filter<float> proxCompIIR;
    
    float* nBandsPtr;
    float* xOverFreqs[4];
    float* dirFactors[5];
    float oldDirFactors[5];
    float* bandGains[5];
    float oldBandGains[5];
    float* allowBackwardsPattern;
    int doEq;
    float* proxDistance;
    float oldProxDistance;
    float* zeroDelayMode;

    bool soloActive;
    float* soloBand[5];
    float* muteBand[5];
    bool loadingFile;
    bool trackingActive;
    bool trackingDisturber;
    bool disturberRecorded;
    bool signalRecorded;
    int nrBlocksRecorded;
    
    float omniSqSumDist[5], eightSqSumDist[5], omniEightSumDist[5],
          omniSqSumSig[5], eightSqSumSig[5], omniEightSumSig[5];
    
    AudioBuffer<float> filterBankBuffer; // holds filtered data, size: N_CH_IN*5
    AudioBuffer<float> firFilterBuffer; // holds filter coefficients, size: 5
    AudioBuffer<float> omniEightBuffer; // holds omni and fig-of-eight signals, size: 2
    std::vector<std::unique_ptr<dsp::Convolution>> convolvers; // holds nBands (stereo) convolvers
    
    double currentSampleRate;
    int currentBlockSize;
    
    //==============================================================================
    void resetXoverFreqs();
    void computeAllFilterCoefficients();
    void computeFilterCoefficients (int crossoverNr);
    void setProxCompCoefficients(float distance);
    void initAllConvolvers();
    void initConvolver(int convNr);
    void createOmniAndEightSignals (AudioBuffer<float>& buffer);
    void createPolarPatterns (AudioBuffer<float>& buffer);
    void trackSignalEnergy();
    void setMinimumDisturbancePattern();
    void setMaximumSignalPattern();
    void maximizeSigToDistRatio();
    
    // file handling
    File lastDir;
    ScopedPointer<PropertiesFile> properties;
    const String presetProperties[27] = {"nrActiveBands", "xOverF1", "xOverF2", "xOverF3", "xOverF4", "dirFactor1", "dirFactor2", "dirFactor3", "dirFactor4", "dirFactor5", "gain1", "gain2", "gain3", "gain4", "gain5", "solo1", "solo2", "solo3", "solo4", "solo5", "mute1", "mute2", "mute3", "mute4", "mute5","ffDfEq","proximity"};
    
    static const int DF_EQ_LEN = 512;
    static const int FF_EQ_LEN = 512;
    static const int EQ_SAMPLE_RATE = 48000;
};

static const float DFEQ_COEFFS_OMNI[512] = {0.893128,0.226831,-0.305175,0.193475,0.104180,-0.097501,-0.221658,-0.083621,0.113967,0.072912,-0.036269,-0.024528,0.038009,0.019054,-0.045728,-0.038984,-0.016470,-0.000284,-0.016679,-0.020151,-0.010668,0.006171,0.002938,-0.007214,-0.006806,-0.000164,0.003387,0.001469,0.001008,0.001173,-0.002319,-0.006493,-0.010854,-0.007776,-0.004961,0.000228,0.000629,0.000716,-0.003751,-0.008161,-0.012587,-0.013443,-0.010819,-0.006502,-0.002251,-0.000186,-0.000436,-0.003019,-0.007202,-0.010357,-0.012093,-0.010233,-0.008092,-0.004354,-0.002160,0.000321,0.000617,0.000590,-0.000459,-0.001157,-0.001654,-0.001893,-0.001894,-0.002119,-0.002361,-0.002649,-0.002989,-0.003048,-0.003470,-0.003290,-0.003676,-0.003413,-0.003996,-0.004074,-0.004820,-0.005060,-0.005534,-0.005585,-0.005500,-0.005177,-0.004636,-0.004128,-0.003610,-0.003249,-0.003044,-0.002773,-0.002731,-0.002350,-0.002301,-0.001796,-0.001760,-0.001366,-0.001480,-0.001361,-0.001568,-0.001626,-0.001781,-0.001899,-0.001988,-0.002136,-0.002262,-0.002461,-0.002708,-0.002900,-0.003205,-0.003276,-0.003544,-0.003469,-0.003648,-0.003474,-0.003562,-0.003379,-0.003377,-0.003207,-0.003102,-0.002939,-0.002752,-0.002581,-0.002370,-0.002192,-0.002033,-0.001867,-0.001814,-0.001663,-0.001709,-0.001576,-0.001683,-0.001577,-0.001710,-0.001666,-0.001804,-0.001839,-0.001972,-0.002073,-0.002182,-0.002301,-0.002384,-0.002475,-0.002546,-0.002578,-0.002647,-0.002619,-0.002694,-0.002588,-0.002625,-0.002482,-0.002502,-0.002348,-0.002306,-0.002184,-0.002120,-0.002031,-0.001946,-0.001879,-0.001798,-0.001757,-0.001718,-0.001669,-0.001660,-0.001624,-0.001697,-0.001650,-0.001735,-0.001708,-0.001835,-0.001829,-0.001921,-0.001940,-0.002019,-0.002056,-0.002102,-0.002130,-0.002147,-0.002173,-0.002186,-0.002182,-0.002174,-0.002122,-0.002130,-0.002054,-0.002062,-0.001961,-0.001972,-0.001876,-0.001871,-0.001782,-0.001760,-0.001704,-0.001674,-0.001643,-0.001614,-0.001605,-0.001591,-0.001586,-0.001592,-0.001577,-0.001611,-0.001588,-0.001645,-0.001620,-0.001690,-0.001671,-0.001733,-0.001722,-0.001766,-0.001767,-0.001787,-0.001799,-0.001806,-0.001820,-0.001821,-0.001826,-0.001831,-0.001817,-0.001834,-0.001803,-0.001832,-0.001792,-0.001829,-0.001790,-0.001823,-0.001797,-0.001829,-0.001826,-0.001851,-0.001871,-0.001893,-0.001924,-0.001948,-0.001982,-0.002020,-0.002050,-0.002106,-0.002126,-0.002193,-0.002203,-0.002274,-0.002278,-0.002344,-0.002348,-0.002397,-0.002403,-0.002430,-0.002439,-0.002447,-0.002456,-0.002452,-0.002452,-0.002445,-0.002429,-0.002424,-0.002387,-0.002384,-0.002330,-0.002325,-0.002260,-0.002247,-0.002179,-0.002154,-0.002091,-0.002052,-0.001996,-0.001944,-0.001893,-0.001837,-0.001788,-0.001736,-0.001683,-0.001641,-0.001579,-0.001550,-0.001485,-0.001469,-0.001407,-0.001400,-0.001349,-0.001345,-0.001311,-0.001303,-0.001280,-0.001267,-0.001256,-0.001242,-0.001234,-0.001224,-0.001216,-0.001219,-0.001211,-0.001229,-0.001212,-0.001234,-0.001218,-0.001253,-0.001239,-0.001263,-0.001258,-0.001291,-0.001296,-0.001317,-0.001333,-0.001351,-0.001377,-0.001396,-0.001418,-0.001440,-0.001458,-0.001500,-0.001514,-0.001553,-0.001558,-0.001603,-0.001608,-0.001647,-0.001657,-0.001691,-0.001711,-0.001738,-0.001761,-0.001782,-0.001809,-0.001829,-0.001850,-0.001875,-0.001891,-0.001921,-0.001933,-0.001976,-0.001982,-0.002012,-0.002012,-0.002046,-0.002052,-0.002071,-0.002076,-0.002093,-0.002100,-0.002110,-0.002112,-0.002111,-0.002115,-0.002119,-0.002121,-0.002123,-0.002107,-0.002112,-0.002095,-0.002101,-0.002071,-0.002072,-0.002047,-0.002042,-0.002014,-0.001997,-0.001975,-0.001953,-0.001931,-0.001903,-0.001881,-0.001856,-0.001830,-0.001808,-0.001773,-0.001755,-0.001714,-0.001698,-0.001655,-0.001640,-0.001601,-0.001584,-0.001548,-0.001525,-0.001494,-0.001466,-0.001440,-0.001413,-0.001390,-0.001367,-0.001344,-0.001326,-0.001299,-0.001287,-0.001259,-0.001254,-0.001226,-0.001227,-0.001205,-0.001205,-0.001189,-0.001188,-0.001179,-0.001175,-0.001173,-0.001170,-0.001171,-0.001170,-0.001169,-0.001170,-0.001164,-0.001169,-0.001158,-0.001167,-0.001155,-0.001166,-0.001154,-0.001163,-0.001154,-0.001158,-0.001152,-0.001151,-0.001150,-0.001146,-0.001146,-0.001143,-0.001141,-0.001139,-0.001132,-0.001135,-0.001124,-0.001130,-0.001116,-0.001122,-0.001107,-0.001110,-0.001097,-0.001096,-0.001088,-0.001084,-0.001077,-0.001069,-0.001063,-0.001053,-0.001045,-0.001035,-0.001023,-0.001016,-0.000999,-0.000993,-0.000975,-0.000971,-0.000950,-0.000937,-0.000916,-0.000906,-0.000891,-0.000872,-0.000856,-0.000840,-0.000824,-0.000806,-0.000784,-0.000768,-0.000747,-0.000733,-0.000712,-0.000695,-0.000669,-0.000657,-0.000634,-0.000618,-0.000593,-0.000577,-0.000556,-0.000538,-0.000517,-0.000497,-0.000479,-0.000459,-0.000439,-0.000420,-0.000401,-0.000384,-0.000364,-0.000348,-0.000328,-0.000313,-0.000293,-0.000279,-0.000260,-0.000246,-0.000230,-0.000216,-0.000201,-0.000187,-0.000174,-0.000161,-0.000149,-0.000137,-0.000125,-0.000115,-0.000104,-0.000095,-0.000085,-0.000077,-0.000069,-0.000061,-0.000053,-0.000047,-0.000041,-0.000035,-0.000030,-0.000025,-0.000021,-0.000017,-0.000013,-0.000010,-0.000007,-0.000005,-0.000003,-0.000002,-0.000001,-0.000000,-0.000000};

static const float DFEQ_COEFFS_EIGHT[512] = {1.099678,0.020727,-0.218195,0.184517,-0.013301,0.124938,-0.088281,-0.006001,0.053207,0.021356,0.030027,-0.034382,-0.041157,-0.009659,0.010998,0.039171,0.015203,0.012945,-0.009529,-0.000595,0.004295,0.017541,0.014295,0.004369,-0.006579,-0.016732,-0.019793,-0.018894,-0.010698,-0.001548,0.003371,0.003905,-0.002354,-0.003260,-0.006177,-0.001519,-0.001397,0.002962,0.002250,0.003706,0.001933,0.000684,-0.001085,-0.002236,-0.001775,-0.001165,0.000456,0.001712,0.002394,0.003130,0.002068,0.002155,-0.000057,-0.000102,-0.002292,-0.002009,-0.003448,-0.003178,-0.004058,-0.004035,-0.004208,-0.004130,-0.003756,-0.003639,-0.003251,-0.003088,-0.003000,-0.002668,-0.002904,-0.002318,-0.002766,-0.001991,-0.002472,-0.001668,-0.001991,-0.001288,-0.001378,-0.000977,-0.000980,-0.000990,-0.001057,-0.001348,-0.001510,-0.001803,-0.002067,-0.002200,-0.002625,-0.002591,-0.003185,-0.003044,-0.003689,-0.003477,-0.003988,-0.003758,-0.004038,-0.003873,-0.003941,-0.003872,-0.003777,-0.003729,-0.003545,-0.003431,-0.003270,-0.003057,-0.003021,-0.002723,-0.002841,-0.002495,-0.002716,-0.002384,-0.002623,-0.002371,-0.002556,-0.002431,-0.002544,-0.002564,-0.002635,-0.002766,-0.002845,-0.003004,-0.003124,-0.003219,-0.003398,-0.003387,-0.003629,-0.003524,-0.003796,-0.003636,-0.003875,-0.003703,-0.003846,-0.003703,-0.003738,-0.003654,-0.003609,-0.003574,-0.003494,-0.003463,-0.003396,-0.003333,-0.003333,-0.003220,-0.003303,-0.003150,-0.003298,-0.003152,-0.003328,-0.003215,-0.003357,-0.003303,-0.003400,-0.003420,-0.003473,-0.003546,-0.003570,-0.003648,-0.003685,-0.003728,-0.003795,-0.003772,-0.003878,-0.003792,-0.003920,-0.003801,-0.003917,-0.003785,-0.003862,-0.003754,-0.003785,-0.003719,-0.003701,-0.003665,-0.003619,-0.003602,-0.003573,-0.003555,-0.003554,-0.003510,-0.003551,-0.003480,-0.003571,-0.003484,-0.003599,-0.003515,-0.003626,-0.003573,-0.003659,-0.003649,-0.003696,-0.003717,-0.003732,-0.003767,-0.003774,-0.003799,-0.003820,-0.003811,-0.003856,-0.003803,-0.003867,-0.003781,-0.003855,-0.003759,-0.003822,-0.003738,-0.003771,-0.003712,-0.003710,-0.003679,-0.003652,-0.003639,-0.003606,-0.003592,-0.003572,-0.003539,-0.003546,-0.003485,-0.003520,-0.003437,-0.003490,-0.003401,-0.003451,-0.003372,-0.003398,-0.003335,-0.003326,-0.003283,-0.003245,-0.003212,-0.003157,-0.003115,-0.003059,-0.002993,-0.002949,-0.002851,-0.002825,-0.002702,-0.002689,-0.002556,-0.002542,-0.002416,-0.002388,-0.002283,-0.002237,-0.002160,-0.002101,-0.002051,-0.001993,-0.001957,-0.001916,-0.001882,-0.001871,-0.001831,-0.001856,-0.001812,-0.001867,-0.001829,-0.001901,-0.001880,-0.001954,-0.001959,-0.002026,-0.002061,-0.002119,-0.002179,-0.002233,-0.002303,-0.002364,-0.002428,-0.002502,-0.002550,-0.002640,-0.002668,-0.002769,-0.002780,-0.002877,-0.002880,-0.002959,-0.002962,-0.003016,-0.003031,-0.003063,-0.003085,-0.003098,-0.003116,-0.003121,-0.003130,-0.003142,-0.003131,-0.003150,-0.003116,-0.003144,-0.003096,-0.003129,-0.003070,-0.003093,-0.003036,-0.003044,-0.002996,-0.002977,-0.002939,-0.002903,-0.002871,-0.002831,-0.002797,-0.002759,-0.002714,-0.002689,-0.002628,-0.002613,-0.002536,-0.002525,-0.002443,-0.002431,-0.002356,-0.002326,-0.002257,-0.002210,-0.002153,-0.002098,-0.002051,-0.001993,-0.001940,-0.001884,-0.001821,-0.001779,-0.001707,-0.001680,-0.001600,-0.001577,-0.001499,-0.001478,-0.001405,-0.001378,-0.001317,-0.001285,-0.001238,-0.001201,-0.001164,-0.001125,-0.001091,-0.001061,-0.001031,-0.001011,-0.000976,-0.000969,-0.000929,-0.000942,-0.000902,-0.000920,-0.000885,-0.000902,-0.000882,-0.000895,-0.000894,-0.000902,-0.000912,-0.000916,-0.000930,-0.000939,-0.000953,-0.000975,-0.000983,-0.001017,-0.001017,-0.001060,-0.001055,-0.001102,-0.001101,-0.001146,-0.001153,-0.001189,-0.001206,-0.001233,-0.001258,-0.001279,-0.001307,-0.001328,-0.001352,-0.001379,-0.001393,-0.001426,-0.001429,-0.001467,-0.001461,-0.001499,-0.001489,-0.001520,-0.001513,-0.001533,-0.001531,-0.001538,-0.001541,-0.001538,-0.001541,-0.001536,-0.001535,-0.001534,-0.001525,-0.001532,-0.001513,-0.001527,-0.001502,-0.001518,-0.001493,-0.001507,-0.001486,-0.001494,-0.001481,-0.001481,-0.001476,-0.001470,-0.001469,-0.001462,-0.001459,-0.001454,-0.001444,-0.001445,-0.001426,-0.001433,-0.001406,-0.001414,-0.001384,-0.001390,-0.001363,-0.001361,-0.001339,-0.001328,-0.001310,-0.001290,-0.001276,-0.001256,-0.001241,-0.001221,-0.001199,-0.001182,-0.001154,-0.001144,-0.001111,-0.001103,-0.001069,-0.001057,-0.001027,-0.001012,-0.000987,-0.000965,-0.000942,-0.000919,-0.000898,-0.000874,-0.000852,-0.000830,-0.000804,-0.000788,-0.000759,-0.000745,-0.000712,-0.000699,-0.000668,-0.000656,-0.000627,-0.000612,-0.000585,-0.000568,-0.000546,-0.000527,-0.000507,-0.000486,-0.000467,-0.000447,-0.000428,-0.000411,-0.000391,-0.000377,-0.000355,-0.000343,-0.000321,-0.000309,-0.000289,-0.000277,-0.000260,-0.000247,-0.000231,-0.000217,-0.000203,-0.000190,-0.000177,-0.000165,-0.000152,-0.000141,-0.000129,-0.000119,-0.000107,-0.000098,-0.000088,-0.000079,-0.000070,-0.000062,-0.000054,-0.000047,-0.000040,-0.000034,-0.000028,-0.000023,-0.000018,-0.000014,-0.000010,-0.000007,-0.000005,-0.000003,-0.000001,-0.000000,-0.000000};

static const float FFEQ_COEFFS_OMNI[512] = {0.923798,0.118459,0.084478,0.016795,-0.161455,-0.073335,-0.000724,0.005185,0.021870,-0.018465,-0.045974,-0.020138,0.015058,0.040742,0.029334,-0.017267,-0.056054,-0.055077,-0.020787,0.020071,0.032892,0.014205,-0.017589,-0.033386,-0.018737,0.012689,0.034966,0.031579,0.008172,-0.016984,-0.025975,-0.015404,0.002952,0.013997,0.010859,-0.000461,-0.009567,-0.009338,-0.002906,0.002231,0.000415,-0.006530,-0.012210,-0.011394,-0.004245,0.003709,0.006857,0.003191,-0.004137,-0.010197,-0.011560,-0.008325,-0.003426,0.000356,0.001613,0.001306,0.000733,0.001050,0.001936,0.002661,0.002511,0.001487,0.000190,-0.000792,-0.001031,-0.000757,-0.000323,-0.000202,-0.000489,-0.001001,-0.001489,-0.001706,-0.001809,-0.001891,-0.002251,-0.002764,-0.003320,-0.003576,-0.003421,-0.002887,-0.002168,-0.001529,-0.001053,-0.000806,-0.000651,-0.000539,-0.000378,-0.000160,0.000078,0.000337,0.000522,0.000674,0.000710,0.000724,0.000666,0.000605,0.000498,0.000355,0.000181,-0.000025,-0.000208,-0.000401,-0.000575,-0.000779,-0.000990,-0.001191,-0.001350,-0.001414,-0.001418,-0.001343,-0.001274,-0.001180,-0.001115,-0.001023,-0.000920,-0.000778,-0.000606,-0.000423,-0.000224,-0.000054,0.000105,0.000219,0.000311,0.000374,0.000414,0.000442,0.000434,0.000420,0.000364,0.000312,0.000230,0.000150,0.000048,-0.000060,-0.000174,-0.000289,-0.000384,-0.000471,-0.000527,-0.000574,-0.000598,-0.000616,-0.000623,-0.000608,-0.000582,-0.000511,-0.000435,-0.000339,-0.000261,-0.000182,-0.000116,-0.000043,0.000040,0.000114,0.000181,0.000224,0.000258,0.000276,0.000293,0.000295,0.000287,0.000263,0.000216,0.000178,0.000123,0.000072,0.000010,-0.000045,-0.000102,-0.000145,-0.000182,-0.000216,-0.000250,-0.000278,-0.000287,-0.000283,-0.000258,-0.000235,-0.000213,-0.000190,-0.000165,-0.000121,-0.000073,-0.000015,0.000026,0.000065,0.000093,0.000130,0.000168,0.000203,0.000228,0.000232,0.000230,0.000222,0.000223,0.000222,0.000220,0.000207,0.000182,0.000157,0.000127,0.000107,0.000083,0.000068,0.000045,0.000027,0.000008,-0.000007,-0.000017,-0.000026,-0.000029,-0.000036,-0.000037,-0.000042,-0.000042,-0.000040,-0.000039,-0.000036,-0.000040,-0.000039,-0.000047,-0.000050,-0.000062,-0.000077,-0.000100,-0.000127,-0.000156,-0.000189,-0.000222,-0.000263,-0.000304,-0.000352,-0.000397,-0.000444,-0.000491,-0.000537,-0.000586,-0.000629,-0.000674,-0.000708,-0.000744,-0.000773,-0.000801,-0.000823,-0.000841,-0.000854,-0.000860,-0.000865,-0.000862,-0.000858,-0.000848,-0.000835,-0.000817,-0.000792,-0.000764,-0.000728,-0.000693,-0.000652,-0.000613,-0.000567,-0.000521,-0.000470,-0.000419,-0.000370,-0.000319,-0.000271,-0.000220,-0.000170,-0.000119,-0.000072,-0.000028,0.000014,0.000053,0.000092,0.000124,0.000152,0.000170,0.000188,0.000200,0.000213,0.000224,0.000232,0.000238,0.000241,0.000246,0.000248,0.000250,0.000246,0.000237,0.000229,0.000220,0.000214,0.000206,0.000195,0.000175,0.000159,0.000140,0.000126,0.000107,0.000085,0.000058,0.000029,0.000004,-0.000021,-0.000043,-0.000069,-0.000096,-0.000125,-0.000157,-0.000185,-0.000214,-0.000239,-0.000266,-0.000291,-0.000322,-0.000352,-0.000384,-0.000415,-0.000442,-0.000467,-0.000492,-0.000520,-0.000550,-0.000583,-0.000612,-0.000639,-0.000665,-0.000689,-0.000718,-0.000742,-0.000765,-0.000785,-0.000803,-0.000817,-0.000833,-0.000847,-0.000860,-0.000871,-0.000881,-0.000889,-0.000894,-0.000899,-0.000901,-0.000904,-0.000908,-0.000907,-0.000904,-0.000896,-0.000887,-0.000875,-0.000866,-0.000856,-0.000844,-0.000827,-0.000806,-0.000785,-0.000763,-0.000747,-0.000730,-0.000713,-0.000691,-0.000666,-0.000639,-0.000614,-0.000593,-0.000571,-0.000551,-0.000526,-0.000501,-0.000476,-0.000454,-0.000432,-0.000412,-0.000391,-0.000368,-0.000347,-0.000326,-0.000309,-0.000292,-0.000277,-0.000261,-0.000245,-0.000231,-0.000219,-0.000211,-0.000203,-0.000198,-0.000192,-0.000188,-0.000185,-0.000184,-0.000185,-0.000188,-0.000192,-0.000195,-0.000199,-0.000201,-0.000204,-0.000207,-0.000208,-0.000211,-0.000212,-0.000216,-0.000218,-0.000222,-0.000223,-0.000225,-0.000227,-0.000229,-0.000231,-0.000233,-0.000234,-0.000234,-0.000237,-0.000239,-0.000243,-0.000245,-0.000247,-0.000249,-0.000250,-0.000255,-0.000258,-0.000263,-0.000264,-0.000266,-0.000268,-0.000272,-0.000278,-0.000284,-0.000287,-0.000286,-0.000287,-0.000288,-0.000291,-0.000295,-0.000297,-0.000295,-0.000293,-0.000293,-0.000291,-0.000293,-0.000291,-0.000286,-0.000282,-0.000278,-0.000277,-0.000274,-0.000271,-0.000267,-0.000261,-0.000256,-0.000251,-0.000247,-0.000241,-0.000236,-0.000231,-0.000225,-0.000219,-0.000214,-0.000206,-0.000200,-0.000194,-0.000188,-0.000182,-0.000175,-0.000167,-0.000159,-0.000153,-0.000147,-0.000142,-0.000135,-0.000128,-0.000120,-0.000113,-0.000107,-0.000102,-0.000096,-0.000090,-0.000083,-0.000077,-0.000071,-0.000067,-0.000063,-0.000058,-0.000053,-0.000048,-0.000044,-0.000040,-0.000037,-0.000034,-0.000030,-0.000027,-0.000024,-0.000021,-0.000019,-0.000017,-0.000015,-0.000013,-0.000011,-0.000009,-0.000008,-0.000006,-0.000005,-0.000004,-0.000003,-0.000002,-0.000002,-0.000001,-0.000001,-0.000000,-0.000000,-0.000000};

static const float FFEQ_COEFFS_EIGHT[512] = {0.973505,-0.015743,-0.119884,0.017401,0.193473,-0.003962,-0.089810,0.020386,0.083724,0.034610,-0.023902,-0.022050,-0.008386,-0.006994,0.006076,0.028268,0.032291,0.010319,-0.006591,-0.004401,0.009154,0.016144,0.015145,0.008107,-0.002863,-0.012199,-0.016168,-0.013329,-0.006975,0.000669,0.005459,0.005197,0.001403,-0.001754,-0.001986,-0.000479,0.001837,0.003630,0.005002,0.005146,0.004339,0.002790,0.001295,0.000556,0.000532,0.001364,0.002503,0.003788,0.004586,0.004866,0.004548,0.003746,0.002750,0.001657,0.000829,0.000144,-0.000249,-0.000665,-0.000981,-0.001248,-0.001332,-0.001228,-0.001043,-0.000750,-0.000525,-0.000285,-0.000164,-0.000040,0.000062,0.000179,0.000337,0.000471,0.000670,0.000825,0.001065,0.001271,0.001516,0.001686,0.001790,0.001790,0.001673,0.001508,0.001270,0.001063,0.000829,0.000630,0.000412,0.000187,-0.000042,-0.000295,-0.000499,-0.000702,-0.000819,-0.000925,-0.000956,-0.000980,-0.000969,-0.000943,-0.000900,-0.000818,-0.000731,-0.000601,-0.000484,-0.000348,-0.000236,-0.000133,-0.000047,0.000011,0.000069,0.000089,0.000123,0.000121,0.000132,0.000110,0.000084,0.000033,-0.000039,-0.000125,-0.000240,-0.000347,-0.000472,-0.000577,-0.000685,-0.000778,-0.000863,-0.000945,-0.001004,-0.001068,-0.001098,-0.001127,-0.001119,-0.001108,-0.001075,-0.001041,-0.001005,-0.000961,-0.000924,-0.000873,-0.000834,-0.000787,-0.000753,-0.000720,-0.000702,-0.000693,-0.000680,-0.000699,-0.000718,-0.000749,-0.000767,-0.000806,-0.000841,-0.000886,-0.000940,-0.000988,-0.001039,-0.001086,-0.001146,-0.001184,-0.001220,-0.001251,-0.001285,-0.001305,-0.001306,-0.001319,-0.001317,-0.001308,-0.001282,-0.001267,-0.001241,-0.001218,-0.001193,-0.001162,-0.001130,-0.001105,-0.001094,-0.001077,-0.001067,-0.001054,-0.001053,-0.001056,-0.001060,-0.001074,-0.001088,-0.001113,-0.001130,-0.001159,-0.001187,-0.001223,-0.001252,-0.001281,-0.001312,-0.001336,-0.001365,-0.001388,-0.001415,-0.001433,-0.001451,-0.001463,-0.001469,-0.001474,-0.001472,-0.001475,-0.001467,-0.001465,-0.001452,-0.001443,-0.001428,-0.001413,-0.001399,-0.001381,-0.001368,-0.001350,-0.001337,-0.001320,-0.001306,-0.001291,-0.001275,-0.001262,-0.001246,-0.001236,-0.001219,-0.001209,-0.001190,-0.001174,-0.001153,-0.001132,-0.001109,-0.001082,-0.001056,-0.001022,-0.000990,-0.000952,-0.000914,-0.000871,-0.000827,-0.000784,-0.000736,-0.000691,-0.000640,-0.000596,-0.000545,-0.000501,-0.000454,-0.000412,-0.000370,-0.000331,-0.000297,-0.000264,-0.000238,-0.000214,-0.000196,-0.000181,-0.000172,-0.000169,-0.000169,-0.000177,-0.000186,-0.000204,-0.000222,-0.000249,-0.000277,-0.000311,-0.000348,-0.000388,-0.000433,-0.000478,-0.000528,-0.000576,-0.000627,-0.000676,-0.000726,-0.000776,-0.000823,-0.000871,-0.000913,-0.000955,-0.000989,-0.001024,-0.001052,-0.001081,-0.001107,-0.001130,-0.001149,-0.001163,-0.001177,-0.001187,-0.001198,-0.001205,-0.001210,-0.001213,-0.001210,-0.001208,-0.001206,-0.001203,-0.001189,-0.001181,-0.001168,-0.001154,-0.001135,-0.001116,-0.001094,-0.001071,-0.001052,-0.001026,-0.001000,-0.000972,-0.000950,-0.000923,-0.000892,-0.000863,-0.000833,-0.000803,-0.000771,-0.000742,-0.000707,-0.000670,-0.000631,-0.000592,-0.000555,-0.000519,-0.000483,-0.000441,-0.000402,-0.000361,-0.000320,-0.000279,-0.000241,-0.000205,-0.000162,-0.000122,-0.000084,-0.000047,-0.000006,0.000031,0.000068,0.000103,0.000136,0.000169,0.000204,0.000237,0.000263,0.000290,0.000315,0.000342,0.000366,0.000384,0.000401,0.000417,0.000432,0.000442,0.000453,0.000461,0.000466,0.000464,0.000463,0.000462,0.000457,0.000453,0.000446,0.000437,0.000424,0.000412,0.000397,0.000380,0.000365,0.000347,0.000331,0.000310,0.000290,0.000268,0.000246,0.000224,0.000202,0.000179,0.000155,0.000134,0.000110,0.000088,0.000064,0.000043,0.000022,0.000001,-0.000018,-0.000036,-0.000052,-0.000068,-0.000081,-0.000094,-0.000105,-0.000115,-0.000122,-0.000129,-0.000134,-0.000137,-0.000141,-0.000142,-0.000145,-0.000145,-0.000147,-0.000148,-0.000147,-0.000148,-0.000147,-0.000147,-0.000144,-0.000145,-0.000143,-0.000143,-0.000141,-0.000140,-0.000139,-0.000138,-0.000137,-0.000134,-0.000133,-0.000131,-0.000129,-0.000127,-0.000124,-0.000121,-0.000117,-0.000114,-0.000109,-0.000106,-0.000102,-0.000098,-0.000093,-0.000087,-0.000080,-0.000073,-0.000069,-0.000064,-0.000059,-0.000053,-0.000046,-0.000040,-0.000034,-0.000029,-0.000024,-0.000021,-0.000014,-0.000009,-0.000005,-0.000003,0.000002,0.000007,0.000010,0.000013,0.000016,0.000019,0.000022,0.000025,0.000026,0.000026,0.000029,0.000032,0.000032,0.000032,0.000034,0.000034,0.000034,0.000034,0.000035,0.000034,0.000033,0.000032,0.000031,0.000031,0.000031,0.000030,0.000028,0.000027,0.000025,0.000024,0.000022,0.000021,0.000019,0.000018,0.000016,0.000014,0.000013,0.000011,0.000010,0.000009,0.000008,0.000007,0.000005,0.000004,0.000004,0.000003,0.000002,0.000001,0.000001,0.000000,-0.000000,-0.000000,-0.000000,-0.000001,-0.000001,-0.000001,-0.000001,-0.000001,-0.000001,-0.000001,-0.000000,-0.000000,-0.000000,-0.000000,-0.000000,-0.000000,-0.000000,-0.000000};
