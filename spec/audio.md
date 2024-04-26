# Audio Device

Branchy16 will have some number of pannable PDM channels with filter control. PDM, not PCM. Keep reading.

# I/O

For each channel:
 - Pitch
 - Sample start bit
 - Sample end bit
 - Sample loop start bit
 - Sample loop end bit
 - Filter pitch
 - Playback volume L
 - Playback volume R
 - Trigger

Sample addresses are specified as address-and-bit-offset, 20 bits long so two words each. In the future, I may add "audio ROM" functionality which turns these into 32-bit bit offsets into up to a 256MiB address space.

Playback rate is set using one 16-bit value between 0 (11.7hz) and 14336 (768khz), which behaves similarly to a floating point number. The top 4 bits are the octave nibble (2^x), and the next 11 are a note value between 4095 and 2048 (represented by 0-2047). The bottom bit is ignored. Every hardware sample (48khz), the octave is subtracted from the pitch counter. If it is negative, then -floor(C/note) bits are shifted into the PDM bit buffer, which is incremented by the note value times the number of bits shifted in. This way, a single table can be used for notes by setting the bottom 12 (the lowest bit is ignored) bits of the pitch.

Playback volume is just a number 0-65535.

Write 1 to "Trigger" to start playback of a sample, and write 0 to stop it.

# Format

Branchy16's audio is PDM, which is a high-frequency 1-bit sound encoding with error diffusion. It will provide to tools to encode PDM from PCM input.

Note that in PDM, silence is encoded as 01010101010. Reading a stream of just zeroes after an audio bitstream will cause a "clunk" sound.

I've chosen PDM to give branchy16 a unique sound that sounds closer to a cassette tape than old computer hardware, for aesthetic value. For a low-bitrate PDM signal, there will always be some amount of hiss, crunch, and/or mud. An interesting aspect of PDM is that full-volume square waves (or 1-bit decimated signals) are perfectly recreated in PDM, allowing for clean chiptune sounds.

## Storage limits and Quality

In PCM, we have sample rate and bit depth, and the bit rate is the product of those two. With PDM, there is only sample rate, which is the same as the bit rate.

To illustrate the quality levels at play, at 600khz per channel, we get about as much quality as vinyl. At 300khz, we get about an analog cassette of quality, or a dirty vinyl if you like. And at 40khz, we get about a telephone's worth of quality. A more advanced encoder could improve these numbers significantly, but branchy16's is currently only second order.

[More Info](https://en.wikipedia.org/wiki/Delta-sigma_modulation#Theoretical_effective_number_of_bits)

Currently, we have 120kbs of RAM and the audio device shares this. If 40kb is dedicated to audio, that's only 0.05 seconds of CD-quality mono audio, barely enough for a wavetable. However, 40kb can also store a whole second of lo-fi audio, or even 20 seconds of chiptune-quality samples.

If Audio ROM is implemented, that ups the limit to 256MiB, which is enough to store an incredible 30 minutes of stereo, hi-fi sound (by 1970s standards).

## Encoding

Encoding PDM means applying error diffusion and quantization to an oversampled input stream. It's very similar to scaling a greyscale image, then converting it 1-bit color with dithering (setting every pixel to either black or white).

The PDM encoder works on a sliding scale between "hiss" and "crunch", controlled by the drive parameter which reduces error correction. It also benefits from pre-processing audio to raise any low-level sound above the hiss, get rid of excessive bass frequencies, and low-pass away any treble that could cause aliasing.

spec/encode_pdm_1bit.c is a second order delta-sigma PDM encoder. If a DSP nerd is reading this, please help me by writing an Nth order encoder with dithering noise that can ideally still have a "drive" parameter.

## Decoding

Decoding PDM is just a matter of applying a low-pass filter. The filter frequency lets the output quality range from "hiss/crunch" to "mud". It's a lot like taking a dithered 1-bit image with high resolution, then blurring it.

spec/decode_pdm_fast.c is a super-fast PDM decoder using a 128-bit FIR to squeeze out what little quality we have access to.

## Encoder GUI

In the final iteration of branchy16, there will be a GUI to convert audio samples to PDM. It will have the following controls:
 - Input trim and output trim.
 - Quality: about 8khz - 384khz, enabling over or under sampling of the input
 - Drive: 0.0 - 1.0 "drive" control described above.
 - Prefilter: Controllable lowpass and highpass filters to help emphasize parts of the sound.
 - Looping: Set sample loop points.
 - Gain: Turn it too high and you will break the encoder lol
 - Filter preview: Set the FIR filter for listening.
 - Pitch preview: adjust the playback pitch for listening.

It should also be able to import .xi instruments or whatever other people want to add, in addition to any audio format that the browser can decode.
