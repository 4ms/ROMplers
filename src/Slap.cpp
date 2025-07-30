#include "plugin.hpp"
#include "SlapSamples.hpp"
#include <cmath>

struct Slap : Module {
	enum ParamId {
		PITCH_PARAM,
		DECAY_PARAM,
		PUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PITCHCVIN_INPUT,
		DECAYCVIN_INPUT,
		TRIGIN_INPUT,
		VOLCVIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		Slap_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = Slap1;
	int sampleLength = Slap1_len;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float SlapLightBrightness = 0.f;

	Slap() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configSwitch(PITCH_PARAM, 0.f, 4.f, 0.f, "Octave transpose", {"Unison", "+1", "+2", "+3", "+4"});
		configParam(DECAY_PARAM, 0.f, 1.f, 1.f, "Decay", "s");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");

		configInput(PITCHCVIN_INPUT, "Pitch CV (1V/oct)");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(TRIGIN_INPUT, "Trig");
		configInput(VOLCVIN_INPUT, "Volume CV");
		configOutput(AUDIOOUT_OUTPUT, "Audio output");
	}

	void process(const ProcessArgs& args) override {
		float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		float buttonIn = params[PUSH_PARAM].getValue();

		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);

		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;

		bool triggered = trigRising || buttonRising;

		if (triggered) {
			SlapLightBrightness = 1.0f;
			samplePos = 0.f;
			playing = true;
			env = 1.0f;
		}

		SlapLightBrightness = std::max(0.f, SlapLightBrightness - (float)(args.sampleTime * 10.f));
		lights[Slap_LIGHT].setBrightnessSmooth(SlapLightBrightness, args.sampleTime);

		float output = 0.f;

		if (playing && currentSample) {
			// Combine switch knob (octave transpose) with CV (1V/oct)
			float pitchOffset = params[PITCH_PARAM].getValue() + 1.f; 
			float pitchCV = inputs[PITCHCVIN_INPUT].isConnected() ? inputs[PITCHCVIN_INPUT].getVoltage() : 0.f;
			float pitch = pitchOffset + pitchCV;

			float pitchRatio = std::pow(2.f, pitch);

			constexpr float sampleSampleRate = 44100.f;
			float sampleRateRatio = sampleSampleRate / args.sampleRate;

			samplePos += pitchRatio * sampleRateRatio;

			int numSamples = sampleLength / 2;

			if ((int)samplePos >= numSamples) {
				playing = false;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamples) ? idx + 1 : idx;
				float frac = samplePos - idx;

				int16_t s1s = (int16_t)(currentSample[idx * 2] | (currentSample[idx * 2 + 1] << 8));
				int16_t s2s = (int16_t)(currentSample[nextIdx * 2] | (currentSample[nextIdx * 2 + 1] << 8));

				float s1 = (float)s1s / 32768.f;
				float s2 = (float)s2s / 32768.f;

				float sampleValue = s1 + frac * (s2 - s1);

				float decayParam = params[DECAY_PARAM].getValue() + inputs[DECAYCVIN_INPUT].getVoltage();
				decayParam = clamp(decayParam, 0.f, 1.f);

				float minDecayTime = 0.005f;
				float maxDecayTime = (float)numSamples / sampleSampleRate;

				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
				float decayCoef = expf(-1.f / (decayTime * args.sampleRate));

				env *= decayCoef;
				output = sampleValue * env;
			}
		}

		float volumeCV = 5.f;
		if (inputs[VOLCVIN_INPUT].isConnected()) {
			volumeCV = clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f);
		}

		output *= volumeCV / 5.f;
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.0f);
	}
};

struct SlapWidget : ModuleWidget {
	SlapWidget(Slap* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Slap_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 23.593)), module, Slap::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 47.625)), module, Slap::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 72.937)), module, Slap::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 72.937)), module, Slap::Slap_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 37.678)), module, Slap::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 63.254)), module, Slap::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 89.077)), module, Slap::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 89.077)), module, Slap::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 102.835)), module, Slap::AUDIOOUT_OUTPUT));
	}
};

Model* modelSlap = createModel<Slap, SlapWidget>("Slap");