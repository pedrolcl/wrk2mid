/*
    Cakewalk to Standard MIDI Files Command Line Utility Translator
    Copyright (C) 2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
//#include <QDebug>
#include <QtMath>
#include <QFileInfo>
#include <QRegularExpression>
#include "sequence.h"

using namespace drumstick::File;

Sequence::Sequence(QObject *parent) : QObject(parent),
    m_smf(nullptr),
    m_wrk(nullptr),
    m_returnCode(EXIT_SUCCESS),
    m_format(1),
    m_ticksDuration(0),
    m_division(-1),
    m_pos(0),
    m_curTrack(0),
    m_beatMax(0),
    m_barCount(0),
    m_beatCount(0),
    m_lowestMidiNote(127),
    m_highestMidiNote(0),
    m_tempo(500000.0),
    m_tempoFactor(1.0),
    m_ticks2millis(0),
    m_duration(0),
    m_lastBeat(0),
    m_beatLength(0),
    m_tick(0),
    m_timeSignatureSet(false),
    m_keySignatureSet(false),
    m_copyrightSet(false)
{
    m_smf = new QSmf(this);
    connect(m_smf, &QSmf::signalSMFError, this, &Sequence::smfErrorHandler);
    connect(m_smf, &QSmf::signalSMFWriteTrack, this, &Sequence::smfTrackHandler);

    m_wrk = new QWrk(this);
    connect(m_wrk, &QWrk::signalWRKError, this, &Sequence::wrkErrorHandler);
    connect(m_wrk, &QWrk::signalWRKUnknownChunk, this, &Sequence::wrkUpdateLoadProgress);
    connect(m_wrk, &QWrk::signalWRKHeader, this, &Sequence::wrkFileHeader);
    connect(m_wrk, &QWrk::signalWRKEnd, this, &Sequence::wrkEndOfFile);
    connect(m_wrk, &QWrk::signalWRKStreamEnd, this, &Sequence::wrkStreamEndEvent);
    connect(m_wrk, &QWrk::signalWRKGlobalVars, this, &Sequence::wrkGlobalVars);
    connect(m_wrk, &QWrk::signalWRKTrack2, this, &Sequence::wrkTrackHeader);
    connect(m_wrk, &QWrk::signalWRKTimeBase, this, &Sequence::wrkTimeBase);
    connect(m_wrk, &QWrk::signalWRKNote, this, &Sequence::wrkNoteEvent);
    connect(m_wrk, &QWrk::signalWRKKeyPress, this, &Sequence::wrkKeyPressEvent);
    connect(m_wrk, &QWrk::signalWRKCtlChange, this, &Sequence::wrkCtlChangeEvent);
    connect(m_wrk, &QWrk::signalWRKPitchBend, this, &Sequence::wrkPitchBendEvent);
    connect(m_wrk, &QWrk::signalWRKProgram, this, &Sequence::wrkProgramEvent);
    connect(m_wrk, &QWrk::signalWRKChanPress, this, &Sequence::wrkChanPressEvent);
    connect(m_wrk, &QWrk::signalWRKSysexEvent, this, &Sequence::wrkSysexEvent);
    connect(m_wrk, &QWrk::signalWRKSysex, this, &Sequence::wrkSysexEventBank);
    connect(m_wrk, &QWrk::signalWRKText2, this, &Sequence::wrkTextEvent);
    connect(m_wrk, &QWrk::signalWRKTimeSig, this, &Sequence::wrkTimeSignatureEvent);
    connect(m_wrk, &QWrk::signalWRKKeySig, this, &Sequence::wrkKeySig);
    connect(m_wrk, &QWrk::signalWRKTempo, this, &Sequence::wrkTempoEvent);
    connect(m_wrk, &QWrk::signalWRKTrackPatch, this, &Sequence::wrkTrackPatch);
    connect(m_wrk, &QWrk::signalWRKComments2, this, &Sequence::wrkComments);
    connect(m_wrk, &QWrk::signalWRKVariableRecord, this, &Sequence::wrkVariableRecord);
    connect(m_wrk, &QWrk::signalWRKNewTrack2, this, &Sequence::wrkNewTrackHeader);
    connect(m_wrk, &QWrk::signalWRKTrackName2, this, &Sequence::wrkTrackName);
    connect(m_wrk, &QWrk::signalWRKTrackVol, this, &Sequence::wrkTrackVol);
    connect(m_wrk, &QWrk::signalWRKTrackBank, this, &Sequence::wrkTrackBank);
    connect(m_wrk, &QWrk::signalWRKSegment2, this, &Sequence::wrkSegment);
    connect(m_wrk, &QWrk::signalWRKChord, this, &Sequence::wrkChord);
    connect(m_wrk, &QWrk::signalWRKExpression2, this, &Sequence::wrkExpression);
    connect(m_wrk, &QWrk::signalWRKMarker2, this, &Sequence::wrkMarker);
    clear();
}

Sequence::~Sequence()
{
    //qDebug() << Q_FUNC_INFO;
    clear();
}

static inline bool eventLessThan(const MIDIEvent* s1, const MIDIEvent *s2)
{
    return s1->tick() < s2->tick();
}

void Sequence::sort(EventsList &list)
{
    //qDebug() << Q_FUNC_INFO << "#events:" << list.count();
    std::stable_sort(list.begin(), list.end(), eventLessThan);
    // Calculate deltas
    long lastEventTicks = 0;
    foreach(MIDIEvent* ev, list) {
        ev->setDelta(ev->tick() - lastEventTicks);
        lastEventTicks = ev->tick();
    }
}

void Sequence::clear()
{
    //qDebug() << Q_FUNC_INFO;
    m_lblName.clear();
    m_ticksDuration = 0;
    m_division = -1;
    m_pos = 0;
    m_tempo = 500000.0;
    m_tick = 0;
    m_lastBeat = 0;
    m_barCount = 0;
    m_beatCount = 0;
    m_beatMax = 4;
    m_lowestMidiNote = 127;
    m_highestMidiNote = 0;
    m_curTrack = 0;
    m_trackMap.clear();
    m_textEvents.clear();
    m_timeSignatureSet = false;
    m_keySignatureSet = false;
    m_copyrightSet = false;
    for(auto it=m_savedSysexEvents.keyBegin(); it != m_savedSysexEvents.keyEnd(); ++it) {
        delete m_savedSysexEvents[*it];
    }
    m_savedSysexEvents.clear();
    for(auto it=m_tracksList.keyBegin(); it != m_tracksList.keyEnd(); ++it) {
        EventsList& list = m_tracksList[*it];
        while (!list.isEmpty()) {
            delete list.takeFirst();
        }
    }
    m_tracksList.clear();
}

bool Sequence::isEmpty()
{
    bool empty = false;
    for(auto it=m_tracksList.keyBegin(); it != m_tracksList.keyEnd(); ++it) {
        EventsList& list = m_tracksList[*it];
        empty |= list.isEmpty();
    }
    //qDebug() << Q_FUNC_INFO << empty;
    return empty;
}

void Sequence::loadPattern(QList<MIDIEvent*> pattern)
{
    clear();
    m_tracksList[0] = pattern;
}

void Sequence::loadFile(const QString& fileName)
{
    QFileInfo finfo(fileName);
    if (finfo.exists()) {
        clear();
        try {
            emit loadingStart(finfo.size());
            QString ext = finfo.suffix().toLower();
            if (ext == "wrk") {
                m_wrk->readFromFile(fileName);
            } else {
                std::cerr << "wrong file type" << std::endl;
                m_returnCode = EXIT_FAILURE;
                return;
            }
            emit loadingFinished();
            for(auto it=m_tracksList.keyBegin(); it!=m_tracksList.keyEnd(); ++it) {
                EventsList& list = m_tracksList[*it];
                //qDebug() << "track:" << *it;
                if (!list.isEmpty()) {
                    sort(list);
                }
            }
            m_lblName = finfo.fileName();
            m_currentFile = finfo.fileName();
        } catch (...) {
            m_returnCode = EXIT_FAILURE;
            std::cerr << "corrupted file" << std::endl;
            clear();
        }
    }
}

void Sequence::saveFile(const QString& fileName)
{
    int tracks = m_format == 0 ? 1 : m_tracksList.size();
    //qDebug() << Q_FUNC_INFO << fileName << "tracks:" << tracks << m_tracksList.keys();
    m_smf->setDivision(m_division);
    m_smf->setFileFormat(m_format);
    m_smf->setTracks(tracks);
    m_smf->writeToFile(fileName);
}

void Sequence::setOutputFormat(int outputType)
{
    m_format = outputType;
}

int Sequence::returnCode()
{
    return m_returnCode;
}

QString Sequence::currentFile() const
{
    return m_currentFile;
}

void Sequence::timeCalculations()
{
    m_ticks2millis = m_tempo / (1000.0 * m_division * m_tempoFactor);
    //qDebug() << Q_FUNC_INFO << "tempo:" << m_tempo << "div:" << m_division << "ticks2millis:" << m_ticks2millis;
}

qreal Sequence::tempoFactor() const
{
    return m_tempoFactor;
}

void Sequence::setTempoFactor(const qreal factor)
{
    if (m_tempoFactor != factor && factor >= 0.1 && factor <= 10.0) {
        //qDebug() << Q_FUNC_INFO << factor;
        m_tempoFactor = factor;
        timeCalculations();
    }
}

std::chrono::milliseconds Sequence::deltaTimeOfEvent(MIDIEvent *ev) const
{
    return std::chrono::milliseconds(std::lround(ev->delta() * m_ticks2millis));
}

std::chrono::milliseconds Sequence::timeOfTicks(const int ticks) const
{
    return std::chrono::milliseconds(std::lround(ticks * m_ticks2millis));
}

qreal Sequence::currentTempo() const
{
    return m_tempo / m_tempoFactor;
}

int Sequence::songLengthTicks() const
{
    return m_ticksDuration;
}

void Sequence::updateTempo(qreal newTempo)
{
    if (m_tempo != newTempo) {
        //qDebug() << Q_FUNC_INFO << newTempo;
        m_tempo = newTempo;
        timeCalculations();
    }
}

/* **************************************** *
 * SMF (Standard MIDI file) format handling
 * **************************************** */

void Sequence::outputEvent(MIDIEvent* ev)
{
    static const std::type_info& textId = typeid(TextEvent);
    static const std::type_info& tempoId = typeid(TempoEvent);
    static const std::type_info& timeSigId = typeid(TimeSignatureEvent);
    static const std::type_info& keySigId = typeid(KeySignatureEvent);
    static const std::type_info& sysexId = typeid (SysExEvent);

    if (ev->isChannel()) {
        ChannelEvent* ev2 = static_cast<ChannelEvent*>(ev);
        int chan = ev2->channel();
        switch(ev->status()) {
        case MIDIEvent::MIDI_STATUS_NOTEOFF: {
                NoteOffEvent* event = static_cast<NoteOffEvent*>(ev);
                int key = event->key();
                int vel = event->velocity();
                //qDebug() << ev->tick() << "NoteOff:" << chan << key << vel;
                m_smf->writeMidiEvent(ev->delta(), note_off, chan, key, vel);
            }
            break;
        case MIDIEvent::MIDI_STATUS_NOTEON: {
                NoteOnEvent* event = static_cast<NoteOnEvent*>(ev);
                int vel = event->velocity();
                int key = event->key();
                //qDebug() << ev->tick() << "NoteOn:" << chan << key << vel;
                m_smf->writeMidiEvent(ev->delta(), note_on, chan, key, vel);
            }
            break;
        case MIDIEvent::MIDI_STATUS_KEYPRESURE: {
                KeyPressEvent* event = static_cast<KeyPressEvent*>(ev);
                int vel = event->velocity();
                int key = event->key();
                //qDebug() << event->tick() << "KeyPress:" << chan << key << vel;
                m_smf->writeMidiEvent(ev->delta(), poly_aftertouch, chan, key, vel);
            }
            break;
        case MIDIEvent::MIDI_STATUS_CONTROLCHANGE: {
                ControllerEvent* event = static_cast<ControllerEvent*>(ev);
                int par = event->param();
                int val = event->value();
                //qDebug() << event->tick() << "CtrlChg:" << chan << par << val;
                m_smf->writeMidiEvent(ev->delta(), control_change, chan, par, val);
            }
            break;
        case MIDIEvent::MIDI_STATUS_PROGRAMCHANGE: {
                ProgramChangeEvent* event = static_cast<ProgramChangeEvent*>(ev);
                int pgm = event->program();
                //qDebug() << event->tick() << "PgmChg:" << chan << pgm;
                m_smf->writeMidiEvent(ev->delta(), program_chng, chan, pgm);
            }
            break;
        case MIDIEvent::MIDI_STATUS_CHANNELPRESSURE: {
                ChanPressEvent* event = static_cast<ChanPressEvent*>(ev);
                int val = event->value();
                //qDebug() << event->tick() << "ChanPress:" << chan << val;
                m_smf->writeMidiEvent(ev->delta(), channel_aftertouch, chan, val);
            }
            break;
        case MIDIEvent::MIDI_STATUS_PITCHBEND: {
                PitchBendEvent* event = static_cast<PitchBendEvent*>(ev);
                int val = 8192 + event->value();
                int lsb = val % 0x80;
                int msb = val / 0x80;
                //qDebug() << event->tick() << "Bender:" << chan << val << lsb << msb;
                m_smf->writeMidiEvent(ev->delta(), pitch_wheel, chan, lsb, msb);
            }
            break;
        default:
            //qDebug() << ev->tick() << "unknown channel event status:" << ev->status();
            break;
        }
    } else
    if (ev->isMetaEvent()) {
        if (typeid(*ev) == sysexId) {
            SysExEvent* event = static_cast<SysExEvent*>(ev);
            //qDebug() << event->tick() << "SysEx:"  << event->data().toHex();
            m_smf->writeMidiEvent(ev->delta(), system_exclusive, 0, event->data());
        } else
        if (typeid(*ev) == textId) {
            TextEvent* event = static_cast<TextEvent*>(ev);
            //qDebug() << event->tick() << "Text(" << event->textType() << "): " << event->data();
            m_smf->writeMetaEvent(ev->delta(), event->textType(), event->data());
        } else
        if (typeid(*ev) == tempoId) {
            TempoEvent* event = static_cast<TempoEvent*>(ev);
            auto tempo = event->tempo();
            //qDebug() << ev->tick() << "Tempo:" << tempo << "bpm:" << event->bpm();
            m_smf->writeTempo(ev->delta(), tempo);
        } else
        if (typeid(*ev) == timeSigId) {
            TimeSignatureEvent* event = static_cast<TimeSignatureEvent*>(ev);
            int dd, x = event->denominator();
            for (dd = 0; x > 1; x /= 2) {
                ++dd;
            }
            //qDebug() << ev->tick() << "TimeSignature:" << event->numerator() << "/" << event->denominator() << dd;
            m_smf->writeTimeSignature(ev->delta(), event->numerator(), dd, 24, 8);
        } else
        if (typeid(*ev) == keySigId) {
            KeySignatureEvent* event = static_cast<KeySignatureEvent*>(ev);
            //qDebug() << ev->tick() << "KeySignature:" << event->alterations() << event->minorMode();
            m_smf->writeKeySignature(ev->delta(), event->alterations(), event->minorMode());
        } /*else {
            qDebug() << ev->tick() << "unknown meta event:" << typeid(*ev).name();
        }*/
    } /*else {
        qDebug() << ev->tick() << "unknown event type:" << typeid(*ev).name();
    }*/
}

void Sequence::smfTrackHandler(int track)
{
    if (m_format == 0) {
        track = 0;
    } else {
        auto trkids = m_tracksList.keys();
        if (track < trkids.size()) {
            track = trkids[track];
        } else {
            return;
        }
    }
    if (m_tracksList[track].count() > 0) {
        //Debug() << Q_FUNC_INFO << "track:" << track << "events:" << m_tracksList[track].count() << "channel:" << m_trkChannel[track];
        for(auto it = m_tracksList[track].cbegin(); it != m_tracksList[track].cend(); ++it) {
            MIDIEvent* ev = *it;
            outputEvent(ev);
        }
        // final event
        m_smf->writeMetaEvent(0, end_of_track);
    }
}

void Sequence::smfErrorHandler(const QString& errorStr)
{
    std::cerr << errorStr.toStdString() << " at file offset " << m_smf->getFilePos() << std::endl;
    m_returnCode = EXIT_FAILURE;
}

/* ********************************* *
 * Cakewalk WRK file format handling
 * ********************************* */

void Sequence::wrkUpdateLoadProgress()
{
    emit loadingProgress(m_wrk->getFilePos());
}

void Sequence::appendWRKEvent(long ticks, MIDIEvent* ev)
{
    int t = m_format == 0 ? 0 : m_curTrack;
    ev->setTick(ticks);
    if (ev->tag() <= 0) {
        ev->setTag(t);
    }
    m_tracksList[t].append(ev);
    if (ticks > m_ticksDuration) {
        m_ticksDuration = ticks;
    }
    wrkUpdateLoadProgress();
}

void Sequence::wrkErrorHandler(const QString& errorStr)
{
    std::cerr << errorStr.toStdString() << " at file offset " << m_wrk->getFilePos() << std::endl;
    m_returnCode = EXIT_FAILURE;
}

void Sequence::wrkFileHeader(int verh, int verl)
{
    //qDebug() << Q_FUNC_INFO << verh << verl;
    m_curTrack = 0;
    m_division = 120;
    m_beatLength = m_division;
    m_beatMax = 4;
    m_lastBeat = 0;
    m_beatCount = 1;
    m_barCount = 1;
    m_fileFormat = QString("%1.%2").arg(verh).arg(verl);
    wrkUpdateLoadProgress();
}

void Sequence::wrkTimeBase(int timebase)
{
    //qDebug() << Q_FUNC_INFO << timebase;
    m_division = timebase;
    wrkUpdateLoadProgress();
}

void Sequence::wrkGlobalVars()
{
    //qDebug() << Q_FUNC_INFO;
    wrkKeySig(0, m_wrk->getKeySig());
    wrkUpdateLoadProgress();
}

void Sequence::wrkStreamEndEvent(long time)
{
    if (time > m_ticksDuration) {
        m_ticksDuration = time;
    }
    wrkUpdateLoadProgress();
}

void Sequence::wrkTrackHeader( const QByteArray& name1,
                           const QByteArray& name2,
                           int trackno, int channel,
                           int pitch, int velocity, int /*port*/,
                           bool /*selected*/, bool /*muted*/, bool /*loop*/ )
{
    TrackMapRec rec;
    rec.channel = channel;
    rec.pitch = pitch;
    rec.velocity = velocity;
    rec.nameSet = false;
    m_curTrack = trackno + 1;
    //qDebug() << Q_FUNC_INFO << "track:" << m_curTrack << "name:" << name1 << name2 << "channel:" << channel;
    m_trackMap[m_curTrack] = rec;
    QByteArray trkName = name1 + ' ' + name2;
    trkName = trkName.trimmed();
    if (!trkName.isEmpty()) {
        m_trackMap[m_curTrack].nameSet = true;
        appendWRKmetadata(m_curTrack, 0, TextType::TrackName, trkName);
    }
    wrkUpdateLoadProgress();
}

void Sequence::wrkNoteEvent(int track, long time, int chan, int pitch, int vol, int dur)
{
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : chan;
    int key = qBound(0, pitch + rec.pitch, 127);
    int velocity = qBound(0, vol + rec.velocity, 127);
    //qDebug() << Q_FUNC_INFO << track << time << chan << key << velocity << dur;
    m_highestMidiNote = qMax(pitch, m_highestMidiNote);
    m_lowestMidiNote = qMin(pitch, m_lowestMidiNote);
    MIDIEvent* ev = new NoteOnEvent(channel, key, velocity);
    ev->setTag(track+1);
    appendWRKEvent(time, ev);
    ev = new NoteOffEvent(channel, key, velocity);
    ev->setTag(track+1);
    appendWRKEvent(time + dur, ev);
}

void Sequence::wrkKeyPressEvent(int track, long time, int chan, int pitch, int press)
{
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : chan;
    int key = pitch + rec.pitch;
    //qDebug() << Q_FUNC_INFO << track << time << channel << key << press;
    MIDIEvent* ev = new KeyPressEvent(channel, key, press);
    ev->setTag(track+1);
    appendWRKEvent(time, ev);
}

void Sequence::wrkCtlChangeEvent(int track, long time, int chan, int ctl, int value)
{
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : chan;
    //qDebug() << Q_FUNC_INFO << track << time << channel << ctl << value;
    MIDIEvent* ev = new ControllerEvent(channel, ctl, value);
    ev->setTag(track+1);
    appendWRKEvent(time, ev);
}

void Sequence::wrkPitchBendEvent(int track, long time, int chan, int value)
{
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : chan;
    MIDIEvent* ev = new PitchBendEvent(channel, value);
    ev->setTag(track+1);
    appendWRKEvent(time, ev);
}

void Sequence::wrkProgramEvent(int track, long time, int chan, int patch)
{
    if (patch >= 0 && patch < 128) {
        TrackMapRec rec = m_trackMap[track+1];
        int channel = rec.channel > -1 ? rec.channel : chan;
        MIDIEvent* ev = new ProgramChangeEvent(channel, patch);
        ev->setTag(track+1);
        appendWRKEvent(time, ev);
        //qDebug() << Q_FUNC_INFO << track << time << channel << patch;
    }
}

void Sequence::wrkChanPressEvent(int track, long time, int chan, int press)
{
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : chan;
    MIDIEvent* ev = new ChanPressEvent(channel, press);
    ev->setTag(track+1);
    appendWRKEvent(time, ev);
}

void Sequence::wrkSysexEvent(int track, long time, int bank)
{
    Q_UNUSED(track)
    //qDebug() << Q_FUNC_INFO;
    if (m_savedSysexEvents.contains(bank)) {
        SysExEvent *ev = m_savedSysexEvents[bank]->clone();
        appendWRKEvent(time, ev);
    }
    wrkUpdateLoadProgress();
}

void Sequence::wrkSysexEventBank(int bank, const QString& name,
        bool autosend, int port, const QByteArray& data)
{
    Q_UNUSED(name)
    Q_UNUSED(port)
    //qDebug() << Q_FUNC_INFO << bank << name << autosend << data;
    SysExEvent* ev = new SysExEvent(data);
    if (autosend) {
        auto savedTrack = m_curTrack;
        m_curTrack = 0;
        appendWRKEvent(0, ev->clone());
        m_curTrack = savedTrack;
        delete ev;
    } else {
        m_savedSysexEvents[bank] = ev;
    }
    wrkUpdateLoadProgress();
}

void Sequence::appendWRKmetadata(int track, long time, TextType type, const QByteArray& data)
{
    TextEvent *ev = new TextEvent(data, type);
    ev->setTag(track);
    appendWRKEvent(time, ev);
    wrkUpdateLoadProgress();
}

void Sequence::wrkTextEvent(int track, long time, int /*type*/, const QByteArray &data)
{
    //qDebug() << "track:" << track+1 << "time:" << time << "type:" << type << "data:" << data;
    appendWRKmetadata(track+1, time, TextType::Lyric, data);
}

void Sequence::wrkComments(const QByteArray &cmt)
{
    appendWRKmetadata(1, 0, TextType::Text, cmt);
}

void Sequence::wrkVariableRecord(const QString &name, const QByteArray &data)
{
    bool isReadable = (name == "Title" || name == "Author" ||
                       name == "Copyright" || name == "Subtitle" ||
                       name == "Instructions" || name == "Keywords");
    if (isReadable) {
        TextType type = TextType::None;
        if ( name == "Title" || name == "Subtitle" ) {
            type = TextType::TrackName;
        } else if ( name == "Copyright" || name == "Author" ) {
            if (!m_copyrightSet) {
                type = TextType::Copyright;
                m_copyrightSet = true;
            }
        } else {
            type = TextType::Text;
        }

        if (type != TextType::None) {
            appendWRKmetadata(0, 0, type, data);
        }
    }
    wrkUpdateLoadProgress();
}

void Sequence::wrkTempoEvent(long time, int tempo)
{
    double bpm = tempo / 100.0;
    TempoEvent* ev = new TempoEvent(qRound ( 6e7 / bpm ) );
    //qDebug() << Q_FUNC_INFO << "Tempo:" << ev->tempo() << "bpm:" << bpm;
    appendWRKEvent(time, ev);
    if (time == 0) {
        updateTempo(bpm);
    }
}

void Sequence::wrkTrackPatch(int track, int patch)
{
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : 0;
    wrkProgramEvent(track+1, 0, channel, patch);
    //qDebug() << Q_FUNC_INFO << track << patch;
}

void Sequence::wrkNewTrackHeader( const QByteArray& data,
                              int trackno, int channel,
                              int pitch, int velocity, int /*port*/,
                              bool /*selected*/, bool /*muted*/, bool /*loop*/ )
{
    TrackMapRec rec;
    rec.channel = channel;
    rec.pitch = pitch;
    rec.velocity = velocity;
    rec.nameSet = false;
    m_curTrack = trackno + 1;
    //qDebug() << Q_FUNC_INFO << "track:" << m_curTrack << "name:" << data << "channel: " << channel;
    m_trackMap[m_curTrack] = rec;
    appendWRKmetadata(m_curTrack, 0, TextType::TrackName, data);
    wrkUpdateLoadProgress();
}

void Sequence::wrkTrackName(int trackno, const QByteArray &data)
{
    if (!m_trackMap[m_curTrack].nameSet) {
        m_trackMap[m_curTrack].nameSet = true;
        appendWRKmetadata(trackno+1, 0, TextType::TrackName, data);
    }
}

void Sequence::wrkTrackVol(int track, int vol)
{
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track+1];
    int channel = (rec.channel > -1) ? rec.channel : 0;
    //qDebug() << Q_FUNC_INFO << track << channel << vol;
    if (vol < 128) {
        wrkCtlChangeEvent(track, 0, channel, ControllerEvent::MIDI_CTL_MSB_MAIN_VOLUME, vol);
    } else {
        lsb = vol % 0x80;
        msb = vol / 0x80;
        wrkCtlChangeEvent(track, 0, channel, ControllerEvent::MIDI_CTL_LSB_MAIN_VOLUME, lsb);
        wrkCtlChangeEvent(track, 0, channel, ControllerEvent::MIDI_CTL_MSB_MAIN_VOLUME, msb);
    }
}

void Sequence::wrkTrackBank(int track, int bank)
{
    // assume GM/GS bank method
    int lsb, msb;
    TrackMapRec rec = m_trackMap[track+1];
    int channel = rec.channel > -1 ? rec.channel : 0;
    lsb = bank % 0x80;
    msb = bank / 0x80;
    wrkCtlChangeEvent(track+1, 0, channel, ControllerEvent::MIDI_CTL_MSB_BANK, msb);
    wrkCtlChangeEvent(track+1, 0, channel, ControllerEvent::MIDI_CTL_LSB_BANK, lsb);
}

void Sequence::wrkSegment(int track, long time, const QByteArray &name)
{
    if (!name.isEmpty()) {
        appendWRKmetadata(track+1, time, TextType::Marker, name);
    }
}

void Sequence::wrkChord(int track, long time, const QString &name, const QByteArray& /*data*/)
{
    QByteArray data = name.toUtf8();
    appendWRKmetadata(track+1, time, TextType::Cue, data);
}

void Sequence::wrkExpression(int track, long time, int /*code*/, const QByteArray &text)
{
    appendWRKmetadata(track+1, time, TextType::Cue, text);
}

void Sequence::wrkTimeSignatureEvent(int bar, int num, int den)
{
    if (!m_timeSignatureSet) {
        MIDIEvent* ev = new TimeSignatureEvent(num, den);
        m_beatMax = num;
        m_beatLength = m_division * 4 / den;

        TimeSigRec newts;
        newts.bar = bar;
        newts.num = num;
        newts.den = den;
        newts.time = 0;
        if (m_bars.isEmpty()) {
            m_bars.append(newts);
        } else {
            bool found = false;
            foreach(const TimeSigRec& ts, m_bars) {
                if (ts.bar == bar) {
                    newts.time = ts.time;
                    found = true;
                    break;
                }
            }
            if (!found) {
                TimeSigRec& lasts = m_bars.last();
                newts.time = lasts.time +
                        (lasts.num * 4 * m_division / lasts.den * (bar - lasts.bar));
                m_bars.append(newts);
            }
        }
        ev->setTag( bar );
        appendWRKEvent(newts.time, ev);
        //qDebug() << Q_FUNC_INFO << newts.time << bar << num << den;
        m_timeSignatureSet = true;
    }
}

void Sequence::wrkKeySig(int bar, int alt)
{
    if (!m_keySignatureSet) {
        MIDIEvent *ev = new KeySignatureEvent(alt, false);
        long time = 0;
        foreach(const TimeSigRec& ts, m_bars) {
            if (ts.bar == bar) {
                time = ts.time;
                break;
            }
        }
        appendWRKEvent(time, ev);
        //qDebug() << Q_FUNC_INFO << time << alt;
        m_keySignatureSet = true;
    }
}

void Sequence::wrkMarker(long time, int smpte, const QByteArray &data)
{
    Q_UNUSED(smpte)
    //qDebug() << Q_FUNC_INFO << time << smpte << data;
    if (!data.isEmpty()) {
        appendWRKmetadata(1, time, TextType::Marker, data);
    }
}

void Sequence::wrkEndOfFile()
{
    wrkUpdateLoadProgress();
}
