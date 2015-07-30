//
//  MidiMessage.h
//  ASCTard013_midi2gates
//
//  Created by Ascetic on 16/06/2015.
//  Copyright (c) 2015 asctSoftware. All rights reserved.
//

#ifndef ASCTard013_midi2gates_MidiMessage_h
#define ASCTard013_midi2gates_MidiMessage_h

///
/// @brief	midi return codes from sysex processing
///
typedef enum midiReturnCode
{
    DONOTHING        = 0x00,
    SENDTOCOMPONENT  = 0x01,
    SHORTCOMMAND     = 0x02,
    SYSEXNONREALTIME = 0x03,
    SYSEXREALTIME    = 0x04,
    PLAYCONTROL      = 0x05
}midiReturnCode;



///
/// @brief	Enumeration of MIDI types
///
enum MidiType

{
    InvalidType           = 0x00,    ///< For notifying errors
    NoteOff               = 0x80,    ///< Note Off
    NoteOn                = 0x90,    ///< Note On
    AfterTouchPoly        = 0xA0,    ///< Polyphonic AfterTouch
    ControlChange         = 0xB0,    ///< Control Change / Channel Mode
    ProgramChange         = 0xC0,    ///< Program Change
    AfterTouchChannel     = 0xD0,    ///< Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    ///< Pitch Bend
    SystemExclusive       = 0xF0,    ///< System Exclusive
    TimeCodeQuarterFrame  = 0xF1,    ///< System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    ///< System Common - Song Position Pointer
    SongSelect            = 0xF3,    ///< System Common - Song Select
    TuneRequest           = 0xF6,    ///< System Common - Tune Request
    SysexEnd              = 0xF7,    ///< System Exclusive End
    Clock                 = 0xF8,    ///< System Real Time - Timing Clock
    Start                 = 0xFA,    ///< System Real Time - Start
    Continue              = 0xFB,    ///< System Real Time - Continue
    Stop                  = 0xFC,    ///< System Real Time - Stop
    ActiveSensing         = 0xFE,    ///< System Real Time - Active Sensing
    SystemReset           = 0xFF,    ///< System Real Time - System Reset
};

class midiMessage {
    
public:
    
    midiMessage() :mType(InvalidType),channel(1),data(0),value(0),length(0)
    {};
    
    MidiType mType;///< The type of midi message
    uint8_t channel;///< midi channel, 0 for omni
    uint8_t data;///< velocity value or cc value
    uint8_t value;///< the value (note number, cc value)
    int length;///< length is set internally to determine final message length, do not alter manually
    
    
};

#endif
