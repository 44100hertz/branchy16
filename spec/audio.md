# Audio Device

Branchy16 will have some number of pannable PDM channels with filter control. PDM, not PCM. Keep reading.

# Parameters

For each channel (mapped from RAM):
 - Pitch
 - Sample start bit
 - Sample end bit
 - Sample loop start bit
 - Sample loop end bit
 - Filter pitch
 - Playback volume L
 - Playback volume R
 - Playback enable

Sample addresses are specified as address-and-bit-offset, 20 bits long so two words each. (in theory 256MiB is addressable this way)

Playback rate is set using one 16-bit value which is similar to a floating point number, but optimized for music. The top 4 bits are the octave (2^x, 0 <= x <= 12), and the next 12 are a note value between 0 and 4095. This gives the note value a range such that music drivers can share a uniform note table between octaves for a given wavetable.

Internally, every hardware sample (384khz), the octave is subtracted from the pitch counter. If it is negative, then one bit is shifted into the PDM bit buffer, and the timer is incremented by the note value, else the previous bit is shifted in again. Overall, the pitch formula is (384000 / note) * (2 ^ octave), giving a minimum pitch of 93.77hz and a maximum of 384khz. If the pitch exceeds 384khz, it will be capped.

Playback volume is just a number 0-65535.

# Format

Branchy16's audio is PDM, which is a high-frequency 1-bit sound format with error diffusion. Branchy will provide to tools to encode PDM from PCM input.

Note that in PDM, silence is encoded as 01010101010. Reading a stream of just zeroes after an audio bitstream will cause a "clunk" sound.

I've chosen PDM to give branchy16 a unique sound closer to a cassette tape than old computer hardware, for aesthetic value. For a low-bitrate PDM signal, there will always be some amount of hiss, crunch, and/or mud. Another interesting aspect of PDM is that full-volume square waves (or 1-bit decimated signals) are perfectly recreated in PDM, allowing for clean chiptune sounds.

## Storage limits and Quality

In PCM, we have sample rate and bit depth, and the bit rate is the product of those two. With PDM, there is only sample rate, which is the same as the bit rate.

To illustrate the quality at play, at 600khz per channel, we get about as much quality as vinyl. At 300khz, we get about an analog cassette of quality, or a dirty vinyl if you like. And at 40khz, we get about a telephone's worth of quality.

## Encoding

Encoding PDM means applying error diffusion and quantization to an oversampled input stream. It's very similar to scaling a greyscale image, then converting it 1-bit color with dithering (setting every pixel to either black or white).

### Compile-time PDM Encoder

The PDM encoder will have a sample import GUI with many parameters.

 - Base playback parameters for previewing result (filter, pitch, etc.), which can be written to a table in ROM.
 - Optional 12TET 440 pitch table generator.
 - Input sample trimming
 - Input gain
 - Quality: resampling of input sample.
 - Prefilter: Pre-encoding lowpass filter.
 - Drive: Reduces error correction, increasing stability and amplitude while reducing quality. (similar to allowing a dithered image to clip more in solid black or solid white regions)
 - Dithering noise (replaces patterned harmonic distortion with stable noise)

### Runtime PDM encoder

To allow for runtime audio synthesis or compression/decompression, the audio device will also have a very basic onboard PDM encoder.

## Decoding

PDM is decoded by applying a low-pass to the PDM bitstream. The low pass filter can be used merely as an anti-aliasing filter (at 24khz or 22.05khz), or as a stronger softening/de-hissing filter. This is much like changing the blur level of a dithered image.
