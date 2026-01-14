#include "plugin.hpp"
#include "SinSahnixSamples.hpp"

struct SinSahnix : Module {
    enum ParamId {
        SPEED_PARAM,
        LENGTH_PARAM,
        LOOP_PARAM,
        KICKPUSH_PARAM,
        SNAREPUSH_PARAM,
        TOMLPUSH_PARAM,
        TOMMPUSH_PARAM,
        TOMHPUSH_PARAM,
        CYMPUSH_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        SPEEDCVIN_INPUT,
        LENGTHCVIN_INPUT,
        LOOPCVIN_INPUT,
        KICKTRIGIN_INPUT,
        SNARETRIGIN_INPUT,
        TOMLTRIGIN_INPUT,
        TOMMTRIGIN_INPUT,
        TOMHTRIGIN_INPUT,
        CYMTRIGIN_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        KICKOUT_OUTPUT,
        SNAREOUT_OUTPUT,
        TOMLOUT_OUTPUT,
        TOMMOUT_OUTPUT,
        TOMHOUT_OUTPUT,
        CYMOUT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        KICK_LIGHT,
        SNARE_LIGHT,
        TOML_LIGHT,
        TOMM_LIGHT,
        TOMH_LIGHT,
        CYM_LIGHT,
        LOOP_LIGHT,
        LIGHTS_LEN
    };

    struct Voice {
        bool lastInputTrigger = false;
        bool lastButtonTrigger = false;

        float samplePos = 0.f;
        bool playing = false;
        const unsigned char* rawData = nullptr;
        int sampleLength = 0;
        int outputId = 0;
        int lightId = -1;

        int16_t readSample16(int index) {
            return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
        }
    };

    Voice kickVoice;
    Voice snareVoice;
    Voice tomLoVoice;
    Voice tomMedVoice;
    Voice tomHiVoice;
    Voice cymbalVoice;

    const float SPEED_LOW = 0.05f;
    const float SPEED_HIGH = 2.0f;
    const float LENGTH_MIN = 0.1f;
    const float LENGTH_MAX = 1.0f;

    SinSahnix() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
        configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
        configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

        configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
        configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
        configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
        configSwitch(TOMMPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Mid Trig", {"Off", "On"});
        configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
        configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});

        configInput(SPEEDCVIN_INPUT, "Speed CV");
        configInput(LENGTHCVIN_INPUT, "Length CV");
        configInput(LOOPCVIN_INPUT, "Loop Gate");
        configInput(KICKTRIGIN_INPUT, "Kick Trig");
        configInput(SNARETRIGIN_INPUT, "Snare Trig");
        configInput(TOMLTRIGIN_INPUT, "Tom Lo Trig");
        configInput(TOMMTRIGIN_INPUT, "Tom Mid Trig");
        configInput(TOMHTRIGIN_INPUT, "Tom Hi Trig");
        configInput(CYMTRIGIN_INPUT, "Cymbal Trig");

        configOutput(KICKOUT_OUTPUT, "Kick");
        configOutput(SNAREOUT_OUTPUT, "Snare");
        configOutput(TOMLOUT_OUTPUT, "Tom Lo");
        configOutput(TOMMOUT_OUTPUT, "Tom Mid");
        configOutput(TOMHOUT_OUTPUT, "Tom Hi");
        configOutput(CYMOUT_OUTPUT, "Cymbal");

        kickVoice.rawData = SSKick;
        kickVoice.sampleLength = sizeof(SSKick) / 2;
        kickVoice.outputId = KICKOUT_OUTPUT;
        kickVoice.lightId = KICK_LIGHT;

        snareVoice.rawData = SSSnare;
        snareVoice.sampleLength = sizeof(SSSnare) / 2;
        snareVoice.outputId = SNAREOUT_OUTPUT;
        snareVoice.lightId = SNARE_LIGHT;

        tomLoVoice.rawData = SSTomL;
        tomLoVoice.sampleLength = sizeof(SSTomL) / 2;
        tomLoVoice.outputId = TOMLOUT_OUTPUT;
        tomLoVoice.lightId = TOML_LIGHT;

        tomMedVoice.rawData = SSTomM;
        tomMedVoice.sampleLength = sizeof(SSTomM) / 2;
        tomMedVoice.outputId = TOMMOUT_OUTPUT;
        tomMedVoice.lightId = TOMM_LIGHT;

        tomHiVoice.rawData = SSTomH;
        tomHiVoice.sampleLength = sizeof(SSTomH) / 2;
        tomHiVoice.outputId = TOMHOUT_OUTPUT;
        tomHiVoice.lightId = TOMH_LIGHT;

        cymbalVoice.rawData = SSCymbal;
        cymbalVoice.sampleLength = sizeof(SSCymbal) / 2;
        cymbalVoice.outputId = CYMOUT_OUTPUT;
        cymbalVoice.lightId = CYM_LIGHT;
    }

    bool loopState = false;           // the current loop on/off state
	bool lastLoopButton = false;      // previous frame state for the button
	bool lastLoopCVTrigger = false;   // previous frame state for the CV

    void process(const ProcessArgs& args) override {
        // Cache param values once per frame
        float baseSpeedParam = params[SPEED_PARAM].getValue();
        float knobSpeed = 0.01f + baseSpeedParam * 0.99f;

        float speedCV = std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -5.f, 5.f);
        float speedOffset = (speedCV * 0.1f);  // pre-multiplied 0.5/5 = 0.1
        float normSpeed = std::clamp(knobSpeed + speedOffset, 0.01f, 1.0f);
        float speed = SPEED_LOW + (normSpeed - 0.01f) * ((SPEED_HIGH - SPEED_LOW) * (1.0f / 0.99f));

        // Length (pre-calculated constants)
        float knobLength = params[LENGTH_PARAM].getValue();
        float lengthCV = std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -5.f, 5.f);
        float lengthOffset = lengthCV * 0.1f; // same factor as speed for consistency
        float normLength = std::clamp(knobLength + lengthOffset, 0.1f, 1.0f);
        float lengthRatio = LENGTH_MIN + (normLength - 0.1f) * ((LENGTH_MAX - LENGTH_MIN) * (1.0f / 0.9f));

		// --- Loop mode ---
		bool loopButton = params[LOOP_PARAM].getValue() > 0.5f;
		float loopCV = inputs[LOOPCVIN_INPUT].isConnected() ? inputs[LOOPCVIN_INPUT].getVoltage() : 0.f;
		bool loopButtonRising = loopButton && !lastLoopButton;
		if (loopButtonRising) {
			loopState = !loopState;
		}
		if (!loopState && loopCV > 1.f) {       // OFF → ON with positive CV
			loopState = true;
		} else if (loopState && loopCV < -1.f) { // ON → OFF with negative CV
			loopState = false;
		}
		lastLoopButton = loopButton;
		bool loopEnabled = loopState;
		lights[LOOP_LIGHT].setBrightnessSmooth(loopState, args.sampleTime);

        // Process each voice
        processVoice(args, kickVoice, KICKTRIGIN_INPUT, KICKPUSH_PARAM, speed, lengthRatio, loopEnabled);
        processVoice(args, snareVoice, SNARETRIGIN_INPUT, SNAREPUSH_PARAM, speed, lengthRatio, loopEnabled);
        processVoice(args, tomLoVoice, TOMLTRIGIN_INPUT, TOMLPUSH_PARAM, speed, lengthRatio, loopEnabled);
        processVoice(args, tomMedVoice, TOMMTRIGIN_INPUT, TOMMPUSH_PARAM, speed, lengthRatio, loopEnabled);
        processVoice(args, tomHiVoice, TOMHTRIGIN_INPUT, TOMHPUSH_PARAM, speed, lengthRatio, loopEnabled);
        processVoice(args, cymbalVoice, CYMTRIGIN_INPUT, CYMPUSH_PARAM, speed, lengthRatio, loopEnabled);
    }

    void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId, float speed, float lengthRatio, bool loopEnabled) {
        // Read inputs once
        const bool inputTrigger = inputs[trigInputId].getVoltage() > 1.0f;
        const bool buttonTrigger = params[pushParamId].getValue() > 0.5f;
    
        const bool inputRising = inputTrigger && !voice.lastInputTrigger;
        const bool buttonRising = buttonTrigger && !voice.lastButtonTrigger;
    
        // Track last states
        voice.lastInputTrigger = inputTrigger;
        voice.lastButtonTrigger = buttonTrigger;
    
        bool fired = inputRising || buttonRising; // light trigger
    
        // Start playing on trigger or loop start
        if (fired || (loopEnabled && !voice.playing)) {
            voice.playing = true;
            voice.samplePos = 0.f;
            fired = true; // ensure light blinks for loop restart
        }
    
        if (!voice.playing) {
            outputs[voice.outputId].setVoltage(0.f);
            if (voice.lightId >= 0)
                lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
            return;
        }
    
        const int maxSamplesToPlay = (int)(voice.sampleLength * lengthRatio);
        int idx = (int)voice.samplePos;
    
        if (idx < maxSamplesToPlay) {
            const int16_t sampleInt = voice.readSample16(idx);
            const float sample = (float)sampleInt / 32768.f;
            outputs[voice.outputId].setVoltage(sample * 5.f);
            voice.samplePos += speed;
        } else {
            if (loopEnabled) {
                voice.samplePos = 0.f; 
                fired = true; // 🔴 loop restart triggers light
            } else {
                voice.playing = false;
                outputs[voice.outputId].setVoltage(0.f);
            }
        }
    
        // Light handling: blink on trigger or loop restart
        if (voice.lightId >= 0) {
            if (fired)
                lights[voice.lightId].setBrightness(1.f);
            else
                lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
        }
    }
};    

struct SinSahnixWidget : ModuleWidget {
	SinSahnixWidget(SinSahnix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SinSahnix.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module, SinSahnix::LENGTH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module, SinSahnix::SPEED_PARAM));

        addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.2, 12.45)), module, SinSahnix::LOOP_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.2, 12.45)), module, SinSahnix::LOOP_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 37)), module, SinSahnix::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 37)), module, SinSahnix::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 52)), module, SinSahnix::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 52)), module, SinSahnix::SNARE_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 67)), module, SinSahnix::TOMLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 67)), module, SinSahnix::TOML_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 82)), module, SinSahnix::TOMMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 82)), module, SinSahnix::TOMM_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 97)), module, SinSahnix::TOMHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 97)), module, SinSahnix::TOMH_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112)), module, SinSahnix::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 112)), module, SinSahnix::CYM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module, SinSahnix::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module, SinSahnix::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module, SinSahnix::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module, SinSahnix::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 52.0)), module, SinSahnix::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 67.0)), module, SinSahnix::TOMLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 82.0)), module, SinSahnix::TOMMTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 97.0)), module, SinSahnix::TOMHTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module, SinSahnix::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module, SinSahnix::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 52.0)), module, SinSahnix::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 67.0)), module, SinSahnix::TOMLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 82.0)), module, SinSahnix::TOMMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 97.0)), module, SinSahnix::TOMHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)), module, SinSahnix::CYMOUT_OUTPUT));
	}
};

Model* modelSinSahnix = createModel<SinSahnix, SinSahnixWidget>("SinSahnix");