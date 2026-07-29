#ifndef BASS_LOOKUP_H
#define BASS_LOOKUP_H

// Bass-specific lookup functions defined in piano.h
// (pianomodel.cpp's translation unit provides the definitions)
float getValueBassLoopFiltera1(float index);
float getValueBassLoopFilterb0(float index);
float getValueBassLoopFilterb1(float index);

#endif // BASS_LOOKUP_H
