#include "plugin.hpp"
#include "AyysKingSamples.hpp"

struct SpeedQuantity : ParamQuantity {
	std::string getDisplayValueString() override {
		float v = getValue();
		float display = (v >= 0.f) ? (v + 1.f) : (v - 1.f);
		return string::f("%.3gx", display);
	}
};

struct AyysKing : Module {
	enum ParamId {
		SPEED_PARAM, LENGTH_PARAM, LOOP_PARAM,
		KICKPUSH_PARAM, SNARE1PUSH_PARAM, SNARE2PUSH_PARAM,
		CLOSEDHATPUSH_PARAM, OPENHATPUSH_PARAM,
		BONGO1PUSH_PARAM, BONGO2PUSH_PARAM, BONGO3PUSH_PARAM,
		CLAVEPUSH_PARAM, CYMPUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT, LENGTHCVIN_INPUT, LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT, SNARE1TRIGIN_INPUT, SNARE2TRIGIN_INPUT,
		CLOSEDHATTRIGIN_INPUT, OPENHATTRIGIN_INPUT,
		BONGO1TRIGIN_INPUT, BONGO2TRIGIN_INPUT, BONGO3TRIGIN_INPUT,
		CLAVETRIGIN_INPUT, CYMTRIGIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT, SNARE1OUT_OUTPUT, SNARE2OUT_OUTPUT,
		CLOSEDHATOUT_OUTPUT, OPENHATOUT_OUTPUT,
		BONGO1OUT_OUTPUT, BONGO2OUT_OUTPUT, BONGO3OUT_OUTPUT,
		CLAVEOUT_OUTPUT, CYMOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT, SNARE1_LIGHT, SNARE2_LIGHT,
		CLOSEDHAT_LIGHT, OPENHAT_LIGHT,
		BONGO1_LIGHT, BONGO2_LIGHT, BONGO3_LIGHT,
		CLAVE_LIGHT, CYMBAL_LIGHT, LOOP_LIGHT,
		LIGHTS_LEN
	};

	struct Voice {
		bool lastInputTrigger = false;
		bool lastButtonTrigger = false;
		bool playing = false;

		const unsigned char* rawData = nullptr;
		int sampleLength = 0;
		float samplePos = 0.f;
		int outputId = 0;
		int lightId = -1;

		// 5/32768 scale factor, precomputed once per Voice instance
		static constexpr float scaleFactor = 5.f / 32768.f;

		inline int16_t readSample16(int idx) const {
			return (int16_t)(rawData[2 * idx] | (rawData[2 * idx + 1] << 8));
		}

		inline float getSample(int idx) const {
			return readSample16(idx) * scaleFactor;
		}
	};

	Voice voices[10];

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 3.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;
	

	bool loopState = false;           // the current loop on/off state
	bool lastLoopButton = false;      // previous frame state for the button
	bool lastLoopCVTrigger = false;   // previous frame state for the CV

	AyysKing() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam<SpeedQuantity>(SPEED_PARAM, -1.f, 1.f, 0.f, "Speed");
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

		static const char* labels[10] = {
			"Kick", "Snare 1", "Snare 2", "Closed Hat", "Open Hat",
			"Bongo 1", "Bongo 2", "Bongo 3", "Clave", "Cymbal"
		};
		for (int i = 0; i < 10; i++) {
			configSwitch(KICKPUSH_PARAM + i, 0.f, 1.f, 0.f, string::f("%s Trig", labels[i]), {"Off", "On"});
			configInput(KICKTRIGIN_INPUT + i, string::f("%s Trig", labels[i]));
			configOutput(KICKOUT_OUTPUT + i, labels[i]);
		}

		configInput(SPEEDCVIN_INPUT, "Speed CV");
		configInput(LENGTHCVIN_INPUT, "Length CV");
		configInput(LOOPCVIN_INPUT, "Loop CV");

		voices[0] = createVoice(AKKick,        sizeof(AKKick),        KICKOUT_OUTPUT,      KICK_LIGHT);
		voices[1] = createVoice(AKSnare,       sizeof(AKSnare),       SNARE1OUT_OUTPUT,    SNARE1_LIGHT);
		voices[2] = createVoice(AKSnare2,      sizeof(AKSnare2),      SNARE2OUT_OUTPUT,    SNARE2_LIGHT);
		voices[3] = createVoice(AKClosedHiHat, sizeof(AKClosedHiHat), CLOSEDHATOUT_OUTPUT, CLOSEDHAT_LIGHT);
		voices[4] = createVoice(AKOpenHiHat,   sizeof(AKOpenHiHat),   OPENHATOUT_OUTPUT,   OPENHAT_LIGHT);
		voices[5] = createVoice(AKBongo,       sizeof(AKBongo),       BONGO1OUT_OUTPUT,    BONGO1_LIGHT);
		voices[6] = createVoice(AKBongo2,      sizeof(AKBongo2),      BONGO2OUT_OUTPUT,    BONGO2_LIGHT);
		voices[7] = createVoice(AKBongo3,      sizeof(AKBongo3),      BONGO3OUT_OUTPUT,    BONGO3_LIGHT);
		voices[8] = createVoice(AKWood,        sizeof(AKWood),        CLAVEOUT_OUTPUT,     CLAVE_LIGHT);
		voices[9] = createVoice(AKCrash,       sizeof(AKCrash),       CYMOUT_OUTPUT,       CYMBAL_LIGHT);
	}

	Voice createVoice(const unsigned char* data, size_t size, int outId, int lightId) {
		Voice v;
		v.rawData = data;
		v.sampleLength = size / 2;
		v.outputId = outId;
		v.lightId = lightId;
		return v;
	}

	void processVoice(const ProcessArgs& args, Voice& voice, int trigInputId, int pushParamId,
		float speed, int maxSamples, bool loop) {

const bool trig = inputs[trigInputId].getVoltage() > 1.f;
const bool btn  = params[pushParamId].getValue() > 0.5f;

const bool trigRise = trig && !voice.lastInputTrigger;
const bool btnRise  = btn  && !voice.lastButtonTrigger;
const bool trigger  = trigRise || btnRise || (loop && !voice.playing);

voice.lastInputTrigger = trig;
voice.lastButtonTrigger = btn;

bool loopReset = false; // light-only pulse source

if (trigger) {
voice.playing = true;
voice.samplePos = 0.f;
if (voice.lightId >= 0)
  lights[voice.lightId].setBrightness(1.f);
}

if (voice.playing) {
int idx = (int)voice.samplePos;
if (idx >= maxSamples)
  idx = maxSamples - 1;

outputs[voice.outputId].setVoltage(voice.getSample(idx));
voice.samplePos += speed;

if (voice.samplePos >= maxSamples) {
  if (loop) {
	  voice.samplePos = 0.f;
	  loopReset = true; // detect wrap
  } else {
	  voice.playing = false;
	  outputs[voice.outputId].setVoltage(0.f);
  }
}
} else {
outputs[voice.outputId].setVoltage(0.f);
}

// Light handling
if (voice.lightId >= 0) {
if (loopReset)
  lights[voice.lightId].setBrightness(1.f);
lights[voice.lightId].setBrightnessSmooth(0.f, args.sampleTime);
}
}

void process(const ProcessArgs& args) override {
// --- Speed ---
float speedKnobV = (params[SPEED_PARAM].getValue() + 1.f) * 2.5f; // -1..1 → 0–5V
float speedCVV = 0.f;
if (inputs[SPEEDCVIN_INPUT].isConnected())
speedCVV = std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f; // -10..10 → -5..5
float speedNorm = std::clamp(speedKnobV + speedCVV, 0.f, 5.f) / 5.f; // back to 0–1
float speed = SPEED_LOW + speedNorm * (SPEED_HIGH - SPEED_LOW);

// --- Length ---
float lengthKnobV = params[LENGTH_PARAM].getValue() * 5.f; // 0–1 → 0–5 V
float lengthCVV = 0.f;
if (inputs[LENGTHCVIN_INPUT].isConnected())
lengthCVV = std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -10.f, 10.f) * 0.5f;
float lengthNorm = std::clamp(lengthKnobV + lengthCVV, 0.f, 5.f) / 5.f;
float lengthRatio = LENGTH_MIN + lengthNorm * (LENGTH_MAX - LENGTH_MIN);

		// --- Loop mode ---
		bool loopButton = params[LOOP_PARAM].getValue() > 0.5f;
		float loopCV = inputs[LOOPCVIN_INPUT].isConnected() ? inputs[LOOPCVIN_INPUT].getVoltage() : 0.f;
		bool loopButtonRising = loopButton && !lastLoopButton;
		if (loopButtonRising)
			loopState = !loopState;
		static bool lastGateHigh = false; // keeps track of gate state across calls
		bool gateHigh = loopCV > 0.6f;
		if (gateHigh && !lastGateHigh)
			loopState = !loopState;
		lastGateHigh = gateHigh;

		lastLoopButton = loopButton;

		bool loopEnabled = loopState;
		lights[LOOP_LIGHT].setBrightnessSmooth(loopState, args.sampleTime);

// --- Process each voice individually ---
for (int i = 0; i < 10; i++) {
int voiceMaxSamples = int(voices[i].sampleLength * lengthRatio); // per-voice maxSamples
processVoice(args, voices[i], KICKTRIGIN_INPUT + i, KICKPUSH_PARAM + i, speed, voiceMaxSamples, loopEnabled);
}
}
};

struct AyysKingWidget : ModuleWidget {
	AyysKingWidget(AyysKing* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/AyysKing.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 15.501)), module, AyysKing::LENGTH_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(31.249, 15.501)), module, AyysKing::SPEED_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(49.731, 15.501)), module, AyysKing::LOOP_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(49.731, 15.501)), module, AyysKing::LOOP_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(6.999, 42.002)), module, AyysKing::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(6.999, 42.002)), module, AyysKing::KICK_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(18.75, 42.002)), module, AyysKing::SNARE1PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(18.75, 42.002)), module, AyysKing::SNARE1_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(30.501, 42.002)), module, AyysKing::SNARE2PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(30.501, 42.002)), module, AyysKing::SNARE2_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(42.249, 42.002)), module, AyysKing::CLOSEDHATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(42.249, 42.002)), module, AyysKing::CLOSEDHAT_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.0, 42.002)), module, AyysKing::OPENHATPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(54.0, 42.002)), module, AyysKing::OPENHAT_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(6.999, 84.999)), module, AyysKing::BONGO1PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(6.999, 84.999)), module, AyysKing::BONGO1_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(18.75, 84.999)), module, AyysKing::BONGO2PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(18.75, 84.999)), module, AyysKing::BONGO2_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(30.501, 84.999)), module, AyysKing::BONGO3PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(30.501, 84.999)), module, AyysKing::BONGO3_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(42.249, 84.999)), module, AyysKing::CLAVEPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(42.249, 84.999)), module, AyysKing::CLAVE_LIGHT));
		
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(54.0, 84.999)), module, AyysKing::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(54.0, 84.999)), module, AyysKing::CYMBAL_LIGHT));
		

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 30.801)), module, AyysKing::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.249, 30.801)), module, AyysKing::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(49.731, 30.801)), module, AyysKing::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.999, 56.0)), module, AyysKing::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.75, 56.0)), module, AyysKing::SNARE1TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.501, 56.0)), module, AyysKing::SNARE2TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.249, 56.0)), module, AyysKing::CLOSEDHATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.0, 56.0)), module, AyysKing::OPENHATTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.999, 99.001)), module, AyysKing::BONGO1TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.75, 99.001)), module, AyysKing::BONGO2TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.501, 99.001)), module, AyysKing::BONGO3TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.249, 99.001)), module, AyysKing::CLAVETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.0, 99.001)), module, AyysKing::CYMTRIGIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.999, 70.002)), module, AyysKing::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.75, 70.002)), module, AyysKing::SNARE1OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.501, 70.002)), module, AyysKing::SNARE2OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.249, 70.002)), module, AyysKing::CLOSEDHATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.0, 70.002)), module, AyysKing::OPENHATOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.999, 112.999)), module, AyysKing::BONGO1OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.75, 112.999)), module, AyysKing::BONGO2OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.501, 112.999)), module, AyysKing::BONGO3OUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.249, 112.999)), module, AyysKing::CLAVEOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.0, 112.999)), module, AyysKing::CYMOUT_OUTPUT));
	}
};


Model* modelAyysKing = createModel<AyysKing, AyysKingWidget>("AyysKing");