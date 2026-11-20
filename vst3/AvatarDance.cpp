#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "BinaryData.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#define STB_IMAGE_STATIC
#include "vendor/stb_image.h"
#include <array>
#include <cmath>
#include <map>
#if JUCE_WINDOWS
#include <windows.h>
static BOOL CALLBACK checkWindow(HWND window, LPARAM result)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (pid == GetCurrentProcessId() && IsIconic(window)
        && !GetWindow(window, GW_OWNER) && !(GetWindowLongPtr(window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
    {
        *reinterpret_cast<bool*>(result) = true;
        return FALSE;
    }
    return TRUE;
}
static bool avatarHostMinimized() { bool result = false; EnumWindows(checkWindow, reinterpret_cast<LPARAM>(&result)); return result; }
#elif JUCE_MAC
extern bool avatarHostMinimized();
#else
static bool avatarHostMinimized() { return false; }
#endif

using namespace juce;

struct Skin
{
    Image atlas;
    StringArray names;
    int held = -1;
    Array<int> moves;
    String error;

    static Skin load(const String& source)
    {
        Skin skin;
        if (source == "Kira" || source == "Blue Static")
        {
            const bool kira = source == "Kira";
            skin.atlas = ImageCache::getFromMemory(kira ? BinaryData::Kira_png : BinaryData::BlueStatic_png,
                                                   kira ? BinaryData::Kira_pngSize : BinaryData::BlueStatic_pngSize);
            skin.names = {"Idle","Held","Side Step","Punch","Dip Hop","Headbang","Heel Shuffle"};
            skin.held = 1;
        }
        else
        {
            File image(source);
            auto textFile=image.withFileExtension("txt");
            if (textFile.existsAsFile()) skin.names.addLines(textFile.loadFileAsString());
            skin.names.trim();
            while (!skin.names.isEmpty() && skin.names[skin.names.size()-1].isEmpty())
                skin.names.remove(skin.names.size()-1);
            skin.atlas = ImageFileFormat::loadFrom(image);
            if (!skin.atlas.isValid() && image.hasFileExtension("bmp"))
            {
                MemoryBlock bytes;
                if (image.getSize() <= std::numeric_limits<int>::max() && image.loadFileAsData(bytes))
                {
                    int w=0,h=0,channels=0;
                    if (auto* rgba=stbi_load_from_memory(static_cast<const stbi_uc*>(bytes.getData()),
                                                         (int)bytes.getSize(),&w,&h,&channels,4))
                    {
                        skin.atlas=Image(Image::ARGB,w,h,true);
                        Image::BitmapData pixels(skin.atlas,Image::BitmapData::writeOnly);
                        for (int y=0;y<h;++y)
                            for (int x=0;x<w;++x)
                            {
                                auto* pixel=rgba+4*(size_t(y)*w+x);
                                pixels.setPixelColour(x,y,Colour(pixel[0],pixel[1],pixel[2],pixel[3]));
                            }
                        stbi_image_free(rgba);
                    }
                }
            }
            skin.held = skin.names.size() > 1 ? skin.names.size()-1 : -1;
        }
        if (!skin.atlas.isValid() || skin.names.isEmpty() || skin.names.contains("")
            || skin.atlas.getWidth()%8 || skin.atlas.getHeight()%skin.names.size())
        {
            skin.error = "Check image and matching .txt (one name per row, eight columns).";
            return skin;
        }
        for (int row=0; row<skin.names.size(); ++row)
            if (row != skin.held) skin.moves.add(row);
        return skin;
    }
};

class Dancer final : public Component, public FileDragAndDropTarget
{
public:
    std::function<void(int,int)> moved;
    std::function<void(String)> dropped;
    Skin skin;
    bool held=false, mirror=false, solid=true;
    int row=0, frame=0;
    float blend=0, reflection=0;
    void paint(Graphics& g) override
    {
        if (skin.moves.isEmpty()) return;
        Graphics::ScopedSaveState saved(g);
        if (mirror) g.addTransform(AffineTransform::scale(-1,1).translated((float)getWidth(),0));
        const int h=skin.atlas.getHeight()/skin.names.size(), w=skin.atlas.getWidth()/8;
        // Keep the feet anchored when reflection intensity changes.
        const int bodyHeight=roundToInt(getHeight()/1.32);
        auto draw = [&](int x, float alpha)
        {
            g.setOpacity(alpha);
            g.drawImage(skin.atlas,0,0,getWidth(),bodyHeight,x*w,row*h,w,h);
        };
        draw(frame,1);
        if (blend>0) draw((frame+1)%8,blend);
        if (reflection>0)
        {
            g.addTransform(AffineTransform::scale(1,-0.32f).translated(0,bodyHeight*1.32f));
            draw(frame,reflection*0.50f);
        }
    }
    void mouseDown(const MouseEvent& e) override { held=true; anchor=e.getScreenPosition()-getPosition(); }
    void mouseDrag(const MouseEvent& e) override
    {
        auto p=e.getScreenPosition()-anchor;
        setTopLeftPosition(p);
        if (moved) moved(p.x,p.y);
    }
    void mouseUp(const MouseEvent&) override { held=false; }
    bool isInterestedInFileDrag(const StringArray& paths) override { return paths.size()==1; }
    void filesDropped(const StringArray& paths,int,int) override { if (paths.size()==1 && dropped) dropped(paths[0]); }
private:
    Point<int> anchor;
};

class DanceProcessor final : public AudioProcessor, private Timer
{
public:
    DanceProcessor() : AudioProcessor(BusesProperties().withOutput("Output",AudioChannelSet::stereo(),true))
    {
        settings.set("skin","Blue Static"); settings.set("move",1); settings.set("speed",1.0);
        auto registerParameter=[this](RangedAudioParameter* parameter)
        {
            controls.emplace(parameter->paramID,parameter);
            addParameter(parameter);
        };
        for (auto key : {"mirror","sync","visible","solid","front","desktop","blend"})
        {
            String id(key);
            registerParameter(new AudioParameterBool(ParameterID(id,1),id,
                              id=="visible" || id=="solid" || id=="front"));
        }
        registerParameter(new AudioParameterFloat(ParameterID("speed",1),"Speed",
                          NormalisableRange<float>(0.125f,8.f,0.125f),1.f));
        registerParameter(new AudioParameterFloat(ParameterID("reflect",1),"Reflection",0.f,1.f,0.f));
        startTimerHz(40);
    }
    ~DanceProcessor() override
    {
        stopTimer();
        if (dancer) { MessageManagerLock lock; dancer.reset(); }
    }
    NamedValueSet snapshot() const
    {
        const ScopedLock lock(mutex);
        auto state=settings;
        for (const auto& item : controls)
            state.set(item.first,item.second->convertFrom0to1(item.second->getValue()));
        return state;
    }
    void set(const Identifier& key,var value)
    {
        auto it=controls.find(key.toString());
        if (it!=controls.end())
        {
            auto& parameter=*it->second;
            parameter.beginChangeGesture();
            parameter.setValueNotifyingHost(parameter.convertTo0to1((float)value));
            parameter.endChangeGesture();
            return;
        }
        const ScopedLock lock(mutex);
        settings.set(key,value);
    }
    void chooseSkin(String source)
    {
        auto candidate=Skin::load(source);
        if (candidate.error.isNotEmpty()) { message=candidate.error; return; }
        set("skin",source); set("move",0); loaded.clear(); message.clear();
    }
    String status() const { return message; }
    StringArray moveNames() const
    {
        StringArray names;
        if (dancer) for (int r : dancer->skin.moves) names.add(dancer->skin.names[r]);
        return names;
    }
    const String getName() const override { return "Avatar Dance"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const String getProgramName(int) override { return {}; }
    void changeProgramName(int,const String&) override {}
    bool hasEditor() const override { return true; }
    AudioProcessorEditor* createEditor() override;
    void prepareToPlay(double,int) override { notes.fill(0); order=0; selected.store(-1); }
    void releaseResources() override { playing.store(false); selected.store(-1); }
    void processBlock(AudioBuffer<float>& audio,MidiBuffer& midi) override
    {
        audio.clear();
        bool running=false;
        double position=nextBeat;
        if (auto* head=getPlayHead())
            if (auto p=head->getPosition())
            {
                running=p->getIsPlaying();
                if (auto b=p->getBpm()) bpm.store(*b);
                if (auto ppq=p->getPpqPosition()) position=*ppq;
            }
        const int available=moveCount.load();
        for (const auto meta : midi)
        {
            if (meta.numBytes>3) continue;
            auto m=meta.getMessage();
            if (m.isAllNotesOff() || m.isAllSoundOff()) notes.fill(0);
            if (m.isNoteOnOrOff())
            {
                int n=m.getNoteNumber()-60;
                if (n>=0 && n<available && n<(int)notes.size()) notes[(size_t)n]=m.isNoteOn()?++order:0;
            }
        }
        auto last=std::max_element(notes.begin(),notes.end());
        selected.store(*last ? (int)(last-notes.begin()) : -1);
        midi.clear();
        playing.store(running); beat.store(position);
        stamp.store(Time::getMillisecondCounter());
        nextBeat=position+(running ? audio.getNumSamples()*bpm.load()/(60*getSampleRate()) : 0);
    }
    void getStateInformation(MemoryBlock& dest) override
    {
        XmlElement xml("AvatarDance"); snapshot().copyToXmlAttributes(xml); copyXmlToBinary(xml,dest);
    }
    void setStateInformation(const void* data,int bytes) override
    {
        if (auto xml=getXmlFromBinary(data,bytes); xml && xml->hasTagName("AvatarDance"))
        {
            const ScopedLock lock(mutex);
            settings.setFromXmlAttributes(*xml);
            for (const auto& item : controls)
                if (settings.contains(item.first))
                    item.second->setValueNotifyingHost(item.second->convertTo0to1((float)settings[item.first]));
        }
    }
private:
    std::map<String,RangedAudioParameter*> controls;
    mutable CriticalSection mutex;
    NamedValueSet settings;
    std::unique_ptr<Dancer> dancer;
    std::atomic<bool> playing{false};
    std::atomic<double> beat{0}, bpm{120};
    std::atomic<uint32> stamp{0};
    std::atomic<int> selected{-1},moveCount{6};
    std::array<uint64_t,68> notes{};
    uint64_t order=0;
    double nextBeat=0;
    String loaded,message;
    int active=-1,lastCycle=0;
    bool wasRunning=false;
    void timerCallback() override
    {
        auto s=snapshot();
        if (!dancer)
        {
            dancer=std::make_unique<Dancer>();
            dancer->moved=[this](int x,int y){set("x",x);set("y",y);};
            dancer->dropped=[this](String path){chooseSkin(path);};
        }
        auto& d=*dancer;
        String source=s.getWithDefault("skin","Blue Static").toString();
        if (loaded!=source || !d.skin.atlas.isValid())
        {
            auto skin=Skin::load(source);
            message=skin.error;
            if (skin.error.isNotEmpty()) skin=Skin::load("Blue Static");
            d.skin=std::move(skin); loaded=source; active=-1;
            moveCount.store(d.skin.moves.size());
            const int height=164;
            const double aspect=(d.skin.atlas.getWidth()/8.0)/(d.skin.atlas.getHeight()/double(d.skin.names.size()));
            d.setSize(jlimit(16,768,roundToInt(height*aspect)),roundToInt(height*1.32));
        }
        const bool solid=(bool)s.getWithDefault("solid",true);
        const int flags=ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresKeyPresses
                        | (solid ? 0 : ComponentPeer::windowIgnoresMouseClicks);
        if (!d.isOnDesktop() || d.solid!=solid)
        {
            d.solid=solid; d.held=false;
            d.addToDesktop(flags);
        }
        d.setAlwaysOnTop((bool)s.getWithDefault("front",true));
        auto origin=Point<int>((int)s.getWithDefault("x",80),(int)s.getWithDefault("y",100));
        if (!d.held && d.getPosition()!=origin) d.setTopLeftPosition(origin);
        bool show=(bool)s.getWithDefault("visible",true) && ((bool)s["desktop"] || !avatarHostMinimized());
        d.setVisible(show);
        if (!show) { d.held=false; return; }
        double speed=jlimit(0.125,8.,(double)s.getWithDefault("speed",1.0));
        auto age=Time::getMillisecondCounter()-stamp.load();
        bool running=playing.load() && age<500;
        double phase=(running ? beat.load()+age*bpm.load()/60000.0 : Time::getMillisecondCounterHiRes()*bpm.load()/60000.0)*4*speed;
        int cycle=(int)std::floor(phase/8);
        int note=selected.load();
        int move=jlimit(0,d.skin.moves.size()-1,note>=0?note:(int)s["move"]);
        int wanted=d.skin.moves[move];
        bool holding=d.held && d.skin.held>=0;
        if (!running && note<0) wanted=d.skin.moves[0];
        if (active<0 || !(bool)s["sync"] || lastCycle!=cycle || running!=wasRunning) active=wanted;
        lastCycle=cycle; wasRunning=running;
        d.row=holding?d.skin.held:active;
        d.frame=((int)std::floor(phase)%8+8)%8;
        d.blend=(bool)s["blend"]?(float)(phase-std::floor(phase)):0.f;
        d.mirror=(bool)s["mirror"]; d.reflection=jlimit(0.f,1.f,(float)s["reflect"]);
        d.repaint();
    }
};

class Controls final : public AudioProcessorEditor, private Timer, public FileDragAndDropTarget
{
public:
    explicit Controls(DanceProcessor& p) : AudioProcessorEditor(p), processor(p)
    {
        setOpaque(true);
        setSize(430,280);
        auto addLabel=[this](Label& label,String text,int y)
        {
            label.setText(text,dontSendNotification);
            label.setBounds(14,y,90,26);
            addAndMakeVisible(label);
        };
        addLabel(skinLabel,"Avatar",12); addLabel(moveLabel,"Move",46);
        addLabel(speedLabel,"Speed ×",84); addLabel(reflectLabel,"Reflection",118);
        for (Component* c : std::initializer_list<Component*>{&skins,&moves,&open,&speed,&reflect,&status})
            addAndMakeVisible(c);
        skins.setBounds(105,12,170,26); moves.setBounds(105,46,236,26);
        open.setBounds(284,12,130,26);
        skins.addItem("Blue Static",1); skins.addItem("Kira",2);
        skins.onChange=[this]{ if (skins.getSelectedId()>0 && skins.getSelectedId()<=2) processor.chooseSkin(skins.getText()); };
        moves.onChange=[this]{processor.set("move",moves.getSelectedId()-1);};
        open.onClick=[this] {
            chooser=std::make_unique<FileChooser>("Choose a sprite sheet",File(),"*.png;*.jpg;*.jpeg;*.gif;*.bmp");
            chooser->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles,
                [this](const FileChooser& f){ auto file=f.getResult(); if(file.existsAsFile()) processor.chooseSkin(file.getFullPathName()); });
        };
        speed.setBounds(104,84,309,26); reflect.setBounds(104,118,309,26);
        speed.setRange(0.125,8,0.125); reflect.setRange(0,1,0.01);
        speed.onValueChange=[this]{processor.set("speed",speed.getValue());};
        reflect.onValueChange=[this]{processor.set("reflect",reflect.getValue());};
        for (int i=0;i<7;++i)
        {
            auto& t=toggles[(size_t)i];
            t.setButtonText(captions[i]);
            t.setBounds(12+(i%3)*140,156+(i/3)*27,140,24);
            addAndMakeVisible(t);
            t.onClick=[this,i]{processor.set(keys[i],toggles[(size_t)i].getToggleState());};
        }
        status.setBounds(12,238,405,38);
        timerCallback(); startTimerHz(5);
    }
    void paint(Graphics& g) override { g.fillAll(Colour(0xff252534)); }
    bool isInterestedInFileDrag(const StringArray& files) override {return files.size()==1;}
    void filesDropped(const StringArray& files,int,int) override {if(files.size()==1) processor.chooseSkin(files[0]);}
private:
    DanceProcessor& processor;
    Label skinLabel,moveLabel,speedLabel,reflectLabel,status;
    ComboBox skins,moves;
    TextButton open{"Load sheet…"};
    Slider speed,reflect;
    std::array<ToggleButton,7> toggles;
    const char* keys[7]={"mirror","sync","visible","solid","front","desktop","blend"};
    const char* captions[7]={"Mirror","Sync changes","Visible","Solid","Keep in front","On desktop","Blend"};
    StringArray options;
    std::unique_ptr<FileChooser> chooser;
    void timerCallback() override
    {
        auto s=processor.snapshot();
        auto name=s["skin"].toString();
        skins.setText(File::isAbsolutePath(name)?File(name).getFileName():name,dontSendNotification);
        auto names=processor.moveNames();
        if (options!=names)
        {
            options=names; moves.clear(dontSendNotification);
            moves.addItemList(names,1);
        }
        moves.setSelectedId(jlimit(1,jmax(1,names.size()),1+(int)s["move"]),dontSendNotification);
        speed.setValue((double)s.getWithDefault("speed",1.),dontSendNotification);
        reflect.setValue((double)s["reflect"],dontSendNotification);
        for (int i=0;i<7;++i) toggles[(size_t)i].setToggleState((bool)s[keys[i]],dontSendNotification);
        auto msg=processor.status();
        status.setText(msg.isEmpty()?"MIDI 60 (C3 in Live) selects the first move.":msg,dontSendNotification);
    }
};

AudioProcessorEditor* DanceProcessor::createEditor() { return new Controls(*this); }
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DanceProcessor(); }
