#include "plugin.hpp"
#include "KayArrSamples.hpp"

struct KayArr : Module {
	enum ParamId {
		SPEED_PARAM, LENGTH_PARAM, LOOP_PARAM,
		KICKPUSH_PARAM, SNAREPUSH_PARAM, TOMPUSH_PARAM,
		CLPUSH_PARAM, OHPUSH_PARAM, CLAVEPUSH_PARAM,
		RIMSHOTPUSH_PARAM, COWBELLPUSH_PARAM, CYMPUSH_PARAM,
		CONGAPUSH_PARAM, PARAMS_LEN
	};
	enum InputId {
		SPEEDCVIN_INPUT, LENGTHCVIN_INPUT, LOOPCVIN_INPUT,
		KICKTRIGIN_INPUT, SNARETRIGIN_INPUT, TOMTRIG_INPUT,
		CLTRIG_INPUT, OHTRIG_INPUT, CLAVETRIG_INPUT,
		RIMSHOTTRIG_INPUT, COWBELLTRIG_INPUT, CYMBALTRIG_INPUT,
		CONGATRIG_INPUT, INPUTS_LEN
	};
	enum OutputId {
		KICKOUT_OUTPUT, SNAREOUT_OUTPUT, TOMOUT_OUTPUT,
		CLOUT_OUTPUT, OHOUT_OUTPUT, CLAVEOUT_OUTPUT,
		RIMSHOTOUT_OUTPUT, COWBELLOUT_OUTPUT, CYMBALOUT_OUTPUT,
		CONGAOUT_OUTPUT, OUTPUTS_LEN
	};
	enum LightId {
		KICK_LIGHT, SNARE_LIGHT, TOM_LIGHT,
		CL_LIGHT, OH_LIGHT, CLAVE_LIGHT,
		RIMSHOT_LIGHT, COWBELL_LIGHT, CYMBAL_LIGHT,
		CONGA_LIGHT, LIGHTS_LEN
	};

	struct Voice {
		const uint8_t* rawData = nullptr;
		int sampleLength = 0;
		int outputId = -1;
		int lightId = -1;
		int trigInputId = -1;
		int pushParamId = -1;

		bool lastInputTrigger = false;
		bool lastButtonTrigger = false;
		float samplePos = 0.f;
		bool playing = false;

		inline int16_t readSample16(int index) {
			return (int16_t)(rawData[2 * index] | (rawData[2 * index + 1] << 8));
		}
	};

	Voice voices[10];

	const float SPEED_LOW = 0.05f;
	const float SPEED_HIGH = 3.0f;
	const float LENGTH_MIN = 0.1f;
	const float LENGTH_MAX = 1.0f;

	KayArr() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SPEED_PARAM, 0.f, 1.f, 0.33f, "Speed");
		configParam(LENGTH_PARAM, 0.f, 1.f, 1.f, "Length");
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});

		for (int i = KICKPUSH_PARAM; i <= CONGAPUSH_PARAM; i++)
			configSwitch(i, 0.f, 1.f, 0.f, "Trigger", {"Off", "On"});

		for (int i = SPEEDCVIN_INPUT; i < INPUTS_LEN; i++)
			configInput(i, "Input");

		for (int i = KICKOUT_OUTPUT; i < OUTPUTS_LEN; i++)
			configOutput(i, "Output");

		static const struct {
			const uint8_t* data;
			int dataSize;
			int trigInput;
			int pushParam;
			int output;
			int light;
		} voiceMap[10] = {
			{ KRKick, sizeof(KRKick), KICKTRIGIN_INPUT, KICKPUSH_PARAM, KICKOUT_OUTPUT, KICK_LIGHT },
			{ KRSnare, sizeof(KRSnare), SNARETRIGIN_INPUT, SNAREPUSH_PARAM, SNAREOUT_OUTPUT, SNARE_LIGHT },
			{ KRTom, sizeof(KRTom), TOMTRIG_INPUT, TOMPUSH_PARAM, TOMOUT_OUTPUT, TOM_LIGHT },
			{ KRClosedHat, sizeof(KRClosedHat), CLTRIG_INPUT, CLPUSH_PARAM, CLOUT_OUTPUT, CL_LIGHT },
			{ KROpenHat, sizeof(KROpenHat), OHTRIG_INPUT, OHPUSH_PARAM, OHOUT_OUTPUT, OH_LIGHT },
			{ KRClave, sizeof(KRClave), CLAVETRIG_INPUT, CLAVEPUSH_PARAM, CLAVEOUT_OUTPUT, CLAVE_LIGHT },
			{ KRRimshot, sizeof(KRRimshot), RIMSHOTTRIG_INPUT, RIMSHOTPUSH_PARAM, RIMSHOTOUT_OUTPUT, RIMSHOT_LIGHT },
			{ KRCowbell, sizeof(KRCowbell), COWBELLTRIG_INPUT, COWBELLPUSH_PARAM, COWBELLOUT_OUTPUT, COWBELL_LIGHT },
			{ KRCymbal, sizeof(KRCymbal), CYMBALTRIG_INPUT, CYMPUSH_PARAM, CYMBALOUT_OUTPUT, CYMBAL_LIGHT },
			{ KRConga, sizeof(KRConga), CONGATRIG_INPUT, CONGAPUSH_PARAM, CONGAOUT_OUTPUT, CONGA_LIGHT }
		};

		for (int i = 0; i < 10; i++) {
			voices[i].rawData = voiceMap[i].data;
			voices[i].sampleLength = voiceMap[i].dataSize / 2;
			voices[i].outputId = voiceMap[i].output;
			voices[i].lightId = voiceMap[i].light;
			voices[i].trigInputId = voiceMap[i].trigInput;
			voices[i].pushParamId = voiceMap[i].pushParam;
		}
	}

	void process(const ProcessArgs& args) override {
		const float speedKnob = params[SPEED_PARAM].getValue();
		const float speedCV = std::clamp(inputs[SPEEDCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float speed = SPEED_LOW + ((speedKnob + speedCV / 10.f) - 0.01f) * (SPEED_HIGH - SPEED_LOW);

		const float lengthKnob = params[LENGTH_PARAM].getValue();
		const float lengthCV = std::clamp(inputs[LENGTHCVIN_INPUT].getVoltage(), -5.f, 5.f);
		float lengthRatio = LENGTH_MIN + ((lengthKnob + lengthCV / 10.f) - 0.1f) * (LENGTH_MAX - LENGTH_MIN);

		bool loopButton = params[LOOP_PARAM].getValue() > 0.5f;
		bool loopCV = inputs[LOOPCVIN_INPUT].getVoltage() > 1.f;
		bool loopEnabled = loopButton || loopCV;

		for (int i = 0; i < 10; i++) {
			Voice& v = voices[i];

			bool inputTrig = inputs[v.trigInputId].getVoltage() > 1.f;
			bool buttonTrig = params[v.pushParamId].getValue() > 0.5f;
			bool inputRising = inputTrig && !v.lastInputTrigger;
			bool buttonRising = buttonTrig && !v.lastButtonTrigger;

			bool retrigger = inputRising || buttonRising || (loopEnabled && !v.playing);

			if (retrigger) {
				v.samplePos = 0.f;
				v.playing = true;
			}

			v.lastInputTrigger = inputTrig;
			v.lastButtonTrigger = buttonTrig;

			const int maxSamples = (int)(v.sampleLength * lengthRatio);

			if (v.playing) {
				int idx = (int)v.samplePos;
				if (idx < maxSamples) {
					float sample = v.readSample16(idx) / 32768.f;
					outputs[v.outputId].setVoltage(sample * 5.f);
					v.samplePos += speed;
				} else {
					if (loopEnabled) {
						v.samplePos = 0.f;
					} else {
						v.playing = false;
						outputs[v.outputId].setVoltage(0.f);
					}
				}
			} else {
				outputs[v.outputId].setVoltage(0.f);
			}

			// Efficient light fade
			if (v.lightId >= 0) {
				lights[v.lightId].setBrightnessSmooth(retrigger ? 1.f : 0.f, args.sampleTime);
			}
		}
	}
};
struct KayArrWidget : ModuleWidget {
	KayArrWidget(KayArr* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/KayArr_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(25.4, 21.308)), module, KayArr::SPEED_PARAM));
		addParam(createParamCentered<Davies1900hBlack>(mm2px(Vec(11.24, 33.508)), module, KayArr::LENGTH_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(41.683, 33.508)), module, KayArr::LOOP_PARAM));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 59.14)), module, KayArr::KICKPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 59.14)), module, KayArr::KICK_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 59.14)), module, KayArr::SNAREPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 59.14)), module, KayArr::SNARE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 59.14)), module, KayArr::CLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 59.14)), module, KayArr::CL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 59.14)), module, KayArr::OHPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 59.14)), module, KayArr::OH_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 59.14)), module, KayArr::TOMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 59.14)), module, KayArr::TOM_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(7.0, 94.743)), module, KayArr::CONGAPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(7.0, 94.743)), module, KayArr::CONGA_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(16.49, 94.743)), module, KayArr::CLAVEPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(16.49, 94.743)), module, KayArr::CLAVE_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(25.4, 94.743)), module, KayArr::RIMSHOTPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(25.4, 94.743)), module, KayArr::RIMSHOT_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(35.318, 94.743)), module, KayArr::COWBELLPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(35.318, 94.743)), module, KayArr::COWBELL_LIGHT));

		addParam(createParamCentered<LEDBezel>(mm2px(Vec(44.699, 94.743)), module, KayArr::CYMPUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(44.699, 94.743)), module, KayArr::CYMBAL_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 33.97)), module, KayArr::SPEEDCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.24, 45.349)), module, KayArr::LENGTHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.683, 45.349)), module, KayArr::LOOPCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 71.357)), module, KayArr::KICKTRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 71.357)), module, KayArr::SNARETRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 71.357)), module, KayArr::CLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 71.357)), module, KayArr::OHTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 71.357)), module, KayArr::TOMTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.0, 106.959)), module, KayArr::CONGATRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(16.49, 106.959)), module, KayArr::CLAVETRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4, 106.959)), module, KayArr::RIMSHOTTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.318, 106.959)), module, KayArr::COWBELLTRIG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.699, 106.959)), module, KayArr::CYMBALTRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 83.813)), module, KayArr::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 83.813)), module, KayArr::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 83.813)), module, KayArr::CLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 83.813)), module, KayArr::OHOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 83.813)), module, KayArr::TOMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.0, 118.357)), module, KayArr::CONGAOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(16.49, 118.357)), module, KayArr::CLAVEOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.4, 118.357)), module, KayArr::RIMSHOTOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.318, 118.357)), module, KayArr::COWBELLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.699, 118.357)), module, KayArr::CYMBALOUT_OUTPUT));
	}
};


Model* modelKayArr = createModel<KayArr, KayArrWidget>("KayArr");