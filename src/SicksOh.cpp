#include "plugin.hpp"
#include "SicksOhSamples.hpp"

struct SicksOh : Module {
	enum ParamId {
		SPEED_PARAM,
		LENGTH_PARAM,
		LOOP_PARAM,
		KICKPUSH_PARAM,
		SNAREPUSH_PARAM,
		TOMLPUSH_PARAM,
		TOMHPUSH_PARAM,
		CLPUSH_PARAM,
		OHPUSH_PARAM,
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
		TOMHTRIGIN_INPUT,
		CLTRIGIN_INPUT,
		OHTRIGIN_INPUT,
		CYMTRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		TOMLOUT_OUTPUT,
		TOMHOUT_OUTPUT,
		CLOUT_OUTPUT,
		OHOUT_OUTPUT,
		CYMOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT,
		SNARE_LIGHT,
		TOML_LIGHT,
		TOMH_LIGHT,
		CL_LIGHT,
		OH_LIGHT,
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
	Voice tomHiVoice;
	Voice closedHatVoice;
	Voice openHatVoice;
	Voice cymVoice;

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 1.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	SicksOh() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Speed", "%", 0.f, 100.f);
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

		configSwitch(KICKPUSH_PARAM, 0.f, 1.f, 0.f, "Kick Trig", {"Off", "On"});
		configSwitch(SNAREPUSH_PARAM, 0.f, 1.f, 0.f, "Snare Trig", {"Off", "On"});
		configSwitch(TOMLPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Lo Trig", {"Off", "On"});
		configSwitch(TOMHPUSH_PARAM, 0.f, 1.f, 0.f, "Tom Hi Trig", {"Off", "On"});
		configSwitch(CLPUSH_PARAM, 0.f, 1.f, 0.f, "Closed Hat Trig", {"Off", "On"});
		configSwitch(OHPUSH_PARAM, 0.f, 1.f, 0.f, "Open Hat Trig", {"Off", "On"});
		configSwitch(CYMPUSH_PARAM, 0.f, 1.f, 0.f, "Cymbal Trig", {"Off", "On"});

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");
		configInput(KICKTRIGIN_INPUT, "Kick Trig");
		configInput(SNARETRIGIN_INPUT, "Snare Trig");
		configInput(TOMLTRIGIN_INPUT, "Tom Lo Trig");
		configInput(TOMHTRIGIN_INPUT, "Tom Hi Trig");
		configInput(CLTRIGIN_INPUT, "Closed Hat Trig");
		configInput(OHTRIGIN_INPUT, "Open Hat Trig");
		configInput(CYMTRIGIN_INPUT, "Cymbal Trig");

		configOutput(KICKOUT_OUTPUT, "Kick");
		configOutput(SNAREOUT_OUTPUT, "Snare");
		configOutput(TOMLOUT_OUTPUT, "Tom Lo");
		configOutput(TOMHOUT_OUTPUT, "Tom Hi");
		configOutput(CLOUT_OUTPUT, "Closed Hat");
		configOutput(OHOUT_OUTPUT, "Open Hat");
		configOutput(CYMOUT_OUTPUT, "Cymbal");

		kickVoice.rawData = SOKick;
		kickVoice.sampleLength = sizeof(SOKick) / 2;
		kickVoice.outputId = KICKOUT_OUTPUT;
		kickVoice.lightId = KICK_LIGHT;

		snareVoice.rawData = SOSnare;
		snareVoice.sampleLength = sizeof(SOSnare) / 2;
		snareVoice.outputId = SNAREOUT_OUTPUT;
		snareVoice.lightId = SNARE_LIGHT;

		tomLoVoice.rawData = SOTomL;
		tomLoVoice.sampleLength = sizeof(SOTomL) / 2;
		tomLoVoice.outputId = TOMLOUT_OUTPUT;
		tomLoVoice.lightId = TOML_LIGHT;

		tomHiVoice.rawData = SOTomH;
		tomHiVoice.sampleLength = sizeof(SOTomH) / 2;
		tomHiVoice.outputId = TOMHOUT_OUTPUT;
		tomHiVoice.lightId = TOMH_LIGHT;

		closedHatVoice.rawData = SOClosedHat;
		closedHatVoice.sampleLength = sizeof(SOClosedHat) / 2;
		closedHatVoice.outputId = CLOUT_OUTPUT;
		closedHatVoice.lightId = CL_LIGHT;

		openHatVoice.rawData = SOOpenHat;
		openHatVoice.sampleLength = sizeof(SOOpenHat) / 2;
		openHatVoice.outputId = OHOUT_OUTPUT;
		openHatVoice.lightId = OH_LIGHT;

		cymVoice.rawData = SOCym;
		cymVoice.sampleLength = sizeof(SOCym) / 2;
		cymVoice.outputId = CYMOUT_OUTPUT;
		cymVoice.lightId = CYM_LIGHT;
	}

	bool loopState = false;           // the current loop on/off state
	bool lastLoopButton = false;      // previous frame state for the button
	bool lastLoopCVTrigger = false;   // previous frame state for the CV

	void process(const ProcessArgs& args) override {
		// Precompute speed (scaled with CV), std::clamp and map to range
		float knobSpeed = 0.01f + params[SPEED_PARAM].getValue() * (1.0f - 0.01f);
		float speedCV = std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float normSpeed = std::clamp(knobSpeed + (speedCV / 5.f) * 0.5f, 0.01f, 1.0f);
		float speed = SPEED_LOW + (normSpeed - 0.01f) * ((SPEED_HIGH - SPEED_LOW) / (1.0f - 0.01f));
	
		// Precompute length ratio similarly
		float knobLength = params[LENGTH_PARAM].getValue();
		float lengthCV = std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float normLength = std::clamp(knobLength + (lengthCV / 5.f) * 0.5f, 0.1f, 1.0f);
		float lengthRatio = LENGTH_MIN + (normLength - 0.1f) * ((LENGTH_MAX - LENGTH_MIN) / (1.0f - 0.1f));
	
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
		processVoice(args, tomHiVoice, TOMHTRIGIN_INPUT, TOMHPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, closedHatVoice, CLTRIGIN_INPUT, CLPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, openHatVoice, OHTRIGIN_INPUT, OHPUSH_PARAM, speed, lengthRatio, loopEnabled);
		processVoice(args, cymVoice, CYMTRIGIN_INPUT, CYMPUSH_PARAM, speed, lengthRatio, loopEnabled);
	}
	
void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId,
		float speed, float lengthRatio, bool loopEnabled) {
		bool inputTrig = inputs[trigInputId].getVoltage() > 1.f;
		bool buttonTrig = params[pushParamId].getValue() > 0.5f;

		bool inputRising = inputTrig && !voice.lastInputTrigger;
		bool buttonRising = buttonTrig && !voice.lastButtonTrigger;
		bool fired = inputRising || buttonRising;

		voice.lastInputTrigger = inputTrig;
		voice.lastButtonTrigger = buttonTrig;

		// Start playback if triggered manually or if looping and not playing
		if (fired || (loopEnabled && !voice.playing)) {
		voice.playing = true;
		voice.samplePos = 0.f;
		fired = true; // ensure light fires
		}

		if (!voice.playing) {
		outputs[voice.outputId].setVoltage(0.f);
		return;  // early exit if not playing
		}

		int maxSamples = (int)(voice.sampleLength * lengthRatio);
		int idx = (int)voice.samplePos;

		if (idx < maxSamples) {
		int16_t sample = voice.readSample16(idx);
		float out = (float)sample / 32768.f * 5.f;
		outputs[voice.outputId].setVoltage(out);
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

struct SicksOhWidget : ModuleWidget {
	SicksOhWidget(SicksOh* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/SicksOh.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Knob9mm>(mm2px(Vec(7.751, 12.45)), module, SicksOh::LENGTH_PARAM));
		addParam(createParamCentered<Knob9mm>(mm2px(Vec(27.002, 12.45)), module, SicksOh::SPEED_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.2, 12.45)), module, SicksOh::LOOP_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.2, 12.45)), module, SicksOh::LOOP_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 37.0)), module, SicksOh::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 37.0)), module, SicksOh::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 49.499)), module, SicksOh::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 49.499)), module, SicksOh::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 62.001)), module, SicksOh::TOMLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 62.001)), module, SicksOh::TOML_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 74.5)), module, SicksOh::TOMHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 74.5)), module, SicksOh::TOMH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 86.999)), module, SicksOh::CLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 86.999)), module, SicksOh::CL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 99.502)), module, SicksOh::OHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 99.502)), module, SicksOh::OH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.751, 112.0)), module, SicksOh::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.751, 112.0)), module, SicksOh::CYM_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.751, 26.0)), module, SicksOh::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(27.002, 26.0)), module, SicksOh::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.2, 26.0)), module, SicksOh::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 37.0)), module, SicksOh::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 49.499)), module, SicksOh::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 62.001)), module, SicksOh::TOMLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 74.5)), module, SicksOh::TOMHTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 86.999)), module, SicksOh::CLTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 99.502)), module, SicksOh::OHTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(32.0, 112.0)), module, SicksOh::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 37.0)), module, SicksOh::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 49.499)), module, SicksOh::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 62.001)), module, SicksOh::TOMLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 74.5)), module, SicksOh::TOMHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 86.999)), module, SicksOh::CLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 99.502)), module, SicksOh::OHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(43.998, 112.0)), module, SicksOh::CYMOUT_OUTPUT));
	}
};

Model* modelSicksOh = createModel<SicksOh, SicksOhWidget>("SicksOh");