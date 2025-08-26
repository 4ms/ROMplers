#include "plugin.hpp"
#include "OrchHitsSamples.hpp"
#include <cmath>

struct OrchHits : Module {
	enum ParamId {
		SAMPLE_PARAM,
		PITCH_PARAM,
		DECAY_PARAM,
		PUSH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SAMPLECVIN_INPUT,
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
		ORCHHITS_LIGHT,
		LIGHTS_LEN
	};

	const unsigned char* currentSample = Orch1;
	int sampleLength = Orch1_len;
	float samplePos = 0.f;
	bool playing = false;

	float env = 0.f;
	float lastTrigValue = 0.f;
	float lastButtonValue = 0.f;
	float OrchHitsLightBrightness = 0.f;

	static constexpr float sampleSampleRate = 44100.f;

	int numSamples = 17;

	const float minDecayTime = 0.1f;
	const float maxDecayTime = 5.f; 

	OrchHits() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		std::vector<std::string> sampleChoices;
		for (int i = 1; i <= numSamples; ++i)
			sampleChoices.push_back(std::to_string(i));
		configSwitch(SAMPLE_PARAM, 0.f, (numSamples - 1), 0.f, "Sample", sampleChoices);
		configSwitch(PITCH_PARAM, 0.f, 4.f, 2.f, "Octave transpose", {"-2", "-1", "Unison", "+1", "+2"});
		configParam(DECAY_PARAM, 0.1f, 5.f, 5.f, "Decay", "s");
		configParam(PUSH_PARAM, 0.f, 1.f, 0.f, "Trigger button");
	
		configInput(PITCHCVIN_INPUT, "Pitch CV (1V/oct)");
		configInput(DECAYCVIN_INPUT, "Decay CV");
		configInput(TRIGIN_INPUT, "Trig");
		configInput(VOLCVIN_INPUT, "Volume CV");
		configOutput(AUDIOOUT_OUTPUT, "Audio output");
	}	

	float fastPow2(float x) {
		if (x < 0.f) return 1.f / fastPow2(-x);
		if (x > 8.f) x = 8.f;
		return 1.f + x * 0.69314718f;
	}

	void process(const ProcessArgs& args) override {
		float sampleParam = params[SAMPLE_PARAM].getValue();
		if (inputs[SAMPLECVIN_INPUT].isConnected()) {
			sampleParam += inputs[SAMPLECVIN_INPUT].getVoltage();
		}
		sampleParam = clamp(sampleParam, 0.f, static_cast<float>(numSamples - 1));
		int selectedIndex = static_cast<int>(std::round(sampleParam));
	
		switch (selectedIndex) {
			case 0:  currentSample = Orch1;  sampleLength = Orch1_len;  break;
			case 1:  currentSample = Orch2;  sampleLength = Orch2_len;  break;
			case 2:  currentSample = Orch3;  sampleLength = Orch3_len;  break;
			case 3:  currentSample = Orch4;  sampleLength = Orch4_len;  break;
			case 4:  currentSample = Orch5;  sampleLength = Orch5_len;  break;
			case 5:  currentSample = Orch6;  sampleLength = Orch6_len;  break;
			case 6:  currentSample = Orch7;  sampleLength = Orch7_len;  break;
			case 7:  currentSample = Orch8;  sampleLength = Orch8_len;  break;
			case 8:  currentSample = Orch9;  sampleLength = Orch9_len;  break;
			case 9:  currentSample = Orch10; sampleLength = Orch10_len; break;
			case 10: currentSample = Orch11; sampleLength = Orch11_len; break;
			case 11: currentSample = Orch12; sampleLength = Orch12_len; break;
			case 12: currentSample = Orch13; sampleLength = Orch13_len; break;
			case 13: currentSample = Orch14; sampleLength = Orch14_len; break;
			case 14: currentSample = Orch15; sampleLength = Orch15_len; break;
			case 15: currentSample = Orch16; sampleLength = Orch16_len; break;
			case 16: currentSample = Orch17; sampleLength = Orch17_len; break;
			default: currentSample = Orch1;  sampleLength = Orch1_len;  break;
		}
	
		float trigIn = inputs[TRIGIN_INPUT].getVoltage();
		float buttonIn = params[PUSH_PARAM].getValue();
	
		bool trigRising = (lastTrigValue <= 1.f && trigIn > 1.f);
		bool buttonRising = (lastButtonValue <= 0.5f && buttonIn > 0.5f);
		lastTrigValue = trigIn;
		lastButtonValue = buttonIn;
	
		if (trigRising || buttonRising) {
			OrchHitsLightBrightness = 1.f;
			samplePos = 0.f;
			playing = true;
			env = 1.f;
		}
	
		OrchHitsLightBrightness -= args.sampleTime * 10.f;
		if (OrchHitsLightBrightness < 0.f) OrchHitsLightBrightness = 0.f;
		lights[ORCHHITS_LIGHT].setBrightnessSmooth(OrchHitsLightBrightness, args.sampleTime);
	
		float output = 0.f;
	
		if (playing && currentSample) {
			float pitchKnob = params[PITCH_PARAM].getValue();   
			float octaveOffset = pitchKnob - 2.f;               
			float pitchCV = inputs[PITCHCVIN_INPUT].isConnected() ? inputs[PITCHCVIN_INPUT].getVoltage() : 0.f;
			float totalVolts = octaveOffset + pitchCV;
			float pitchRatio = std::pow(2.f, totalVolts);
			
	
			float sampleRateRatio = sampleSampleRate / args.sampleRate;
			samplePos += pitchRatio * sampleRateRatio;
	
			int numSamplesInSample = sampleLength / 2;
	
			if ((int)samplePos >= numSamplesInSample) {
				playing = false;
				output = 0.f;
			} else {
				int idx = (int)samplePos;
				int nextIdx = (idx + 1 < numSamplesInSample) ? idx + 1 : idx;
				float frac = samplePos - idx;
	
				const unsigned char* d = currentSample;
				int16_t s1s = (int16_t)(d[idx * 2] | (d[idx * 2 + 1] << 8));
				int16_t s2s = (int16_t)(d[nextIdx * 2] | (d[nextIdx * 2 + 1] << 8));
				float s1 = s1s * (1.f / 32768.f);
				float s2 = s2s * (1.f / 32768.f);
	
				float sampleValue = s1 + frac * (s2 - s1);
	
				float knobDecayTime = params[DECAY_PARAM].getValue();
				float decayParam = (knobDecayTime - minDecayTime) / (maxDecayTime - minDecayTime);
				if (inputs[DECAYCVIN_INPUT].isConnected())
					decayParam += inputs[DECAYCVIN_INPUT].getVoltage();
				decayParam = clamp(decayParam, 0.f, 1.f);
				float decayTime = minDecayTime + decayParam * (maxDecayTime - minDecayTime);
	
				float decayCoef = std::exp(-1.f / (decayTime * args.sampleRate));
				env *= decayCoef;
	
				output = sampleValue * env;
			}
		}
	
		float volumeCV = inputs[VOLCVIN_INPUT].isConnected() ? clamp(inputs[VOLCVIN_INPUT].getVoltage(), 0.f, 5.f) : 5.f;
		output *= volumeCV / 5.f;
		outputs[AUDIOOUT_OUTPUT].setVoltage(output * 5.f);
	}	
};

struct OrchHitsWidget : ModuleWidget {
	OrchHitsWidget(OrchHits* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/OrchHits_info.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 16.052)), module, OrchHits::SAMPLE_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 39.468)), module, OrchHits::PITCH_PARAM));
		addParam(createParamCentered<_9mmKnob>(mm2px(Vec(10.16, 63.5)), module, OrchHits::DECAY_PARAM));
		addParam(createParamCentered<LEDBezel>(mm2px(Vec(10.16, 88.812)), module, OrchHits::PUSH_PARAM));
		addChild(createLightCentered<LEDBezelLight<WhiteLight>>(mm2px(Vec(10.16, 88.812)), module, OrchHits::ORCHHITS_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 30.138)), module, OrchHits::SAMPLECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 53.553)), module, OrchHits::PITCHCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 79.129)), module, OrchHits::DECAYCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.469, 104.952)), module, OrchHits::TRIGIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.994, 104.952)), module, OrchHits::VOLCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 118.71)), module, OrchHits::AUDIOOUT_OUTPUT));
	}
};


Model* modelOrchHits = createModel<OrchHits, OrchHitsWidget>("OrchHits");