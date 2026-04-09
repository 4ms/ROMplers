# ROMplers User Manual

Use this document to understand the behavior, controls, and design inspirations behind each module.

---

## CV + Knob Behavior

- All unlabeled jacks are CV inputs for the associated parameter next to them. 
- All CV inputs are bipolar and clamped to 10vpp.
- Each knob functions as a 10vpp offset.
- Trigger inputs are conditioned to have rising edge detection and ignore pulse width.

---
## AyysKing — Ace King drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/AyysKing.png" width="200"></p>

Ace Tone Rhythm King samples with playback speed, sample length, and a mix output.

- **Length**: Sample file length.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## Clap — Acoustic claps

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Clap.png" width="60"></p>

“Real” acoustic clap samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## ClosedHat — Acoustic closed hi-hats

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/ClosedHiHat.png" width="60"></p>

“Real” acoustic closed hi-hat samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## Crash — Acoustic crash cymbals

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Crash.png" width="60"></p>

“Real” acoustic crash samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## DeeArr — DR-55 drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/DeeArr.png" width="150"></p>

DR-55 samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## KayArr — KR-55 drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/KayArr.png" width="150"></p>

KR55 samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## KayOne — SK-1 drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/KayOne.png" width="150"></p>

Casio SK-1 samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## Kick — Acoustic kick drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Kick.png" width="60"></p>

“Real” acoustic kick samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## OpenHat — Acoustic open hi-hats

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/OpenHiHat.png" width="60"></p>

“Real” acoustic open hi-hat samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## OrchHits — Pitched orchestra hits

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/OrchestraHits.png" width="60"></p>

Pitched orchestra hits, meant to be controlled with a 1v per octave source such as a keyboard or sequencer.

- **Sample**: Sample selection. There are 17 total samples for this module.
- **Octave**: Octave transpose for sample, from -2 to +2 octaves above root pitch.
- **1v/Oct**: 1v per octave pitch input.
- **Decay**: Exponential decay envelope 1ms - 5 seconds.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## Percussion — Assorted acoustic hand percussion 

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Percussion.png" width="60"></p>

“Real” acoustic auxiliary percussion samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## Ride — Acoustic ride cymbals

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Ride.png" width="60"></p>

“Real” acoustic ride samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## Rimshot — Acoustic rimshots

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Rimshot.png" width="60"></p>

“Real” acoustic rimshot samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## SeaArr — CR-78 drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/SeaArr.png" width="200"></p>

CR-78 samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## SehvenToo — TR-727 drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/SehvenToo.png" width="200"></p>

TR-727 samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## SicksOh — TR-606 drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/SicksOh.png" width="150"></p>

TR-606 samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## SinSahnix — Synsonics drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/SinSahnix.png" width="150"></p>

Synsonics samples with playback speed, sample length, direct outs, and a mix output.

- **Length**: Sample file length for all samples.
- **Speed**: Playback speed and pitch of the samples. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Main Volume**: Master volume trimpot for the Sum output, 0–100%.
- **Buttons**: Triggers samples.
- **Trigger inputs**: Trigger input, conditioned for rising edge detection.
- **Outputs**: Individual audio outputs, ±5V.
- **Sum**: Mix output. All drums not individually patched are summed at unity gain, averaged, scaled by Main Volume, and clamped to ±5V. Patching a drum's direct output removes it from the Sum bus.

---
## Slap — Pitched slap bass

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Slap.png" width="60"></p>

A slap bass sample meant to be controlled with a 1v per octave source such as a keyboard or sequencer.

- **Octave**: Octave transpose for sample, from Unison to +4v octaves above root pitch.
- **1v/Oct**: 1v per octave pitch input.
- **Decay**: Exponential decay envelope 1ms - 1 second.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## Snare — Acoustic snare drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Snare.png" width="60"></p>

“Real” acoustic snare samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

---
## Tom — Acoustic tom drums

<p align="center"><img src="https://github.com/4ms/ROMplers/blob/main/doc/Tom.png" width="60"></p>

“Real” acoustic tom samples with playback speed, velocity, and a decay envelope.

- **Sample**: Sample selection. There are 16 total samples for this module.
- **Pitch**: Playback speed and pitch of the sample. Center position is 1x (original speed); turn right to increase up to 2x, turn left to decrease to -2x.
- **Decay**: An exponential decay envelope, 1ms - 1 second long.
- **Button**: Triggers the sample.
- **Trig**: Trigger input, conditioned for rising edge detection. 
- **Vol**: VCA input, responds 0-5v. This jack can be viewed as a "velocity" input.
- **Out**: Audio output, 10vpp.

