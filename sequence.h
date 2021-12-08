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

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <drumstick/qsmf.h>
#include <drumstick/qwrk.h>
#include "events.h"

typedef QList<MIDIEvent*> EventsList;

class Sequence : public QObject
{
    Q_OBJECT

public:
    enum TextType {
        None = 0, Text = 1, Copyright = 2, TrackName = 3,
        InstrumentName = 4, Lyric = 5, Marker = 6, Cue = 7,
        FIRST_TYPE = Text, LAST_TYPE = Cue
    };
    Q_ENUM(TextType)

    explicit Sequence(QObject* parent = 0);
    virtual ~Sequence();

    void clear();
    void appendEvent(MIDIEvent* ev);
    void loadPattern(QList<MIDIEvent*> pattern);
    void loadFile(const QString& fileName);
    void saveFile(const QString& fileName);
    void setOutputFormat(int outputType);
    int returnCode();

    qreal tempoFactor() const;
    void setTempoFactor(const qreal factor);
    MIDIEvent *nextEvent();
    int eventTime(MIDIEvent* ev) const;
    std::chrono::milliseconds deltaTimeOfEvent(MIDIEvent* ev) const;
    std::chrono::milliseconds timeOfTicks(const int ticks) const;
    bool hasMoreEvents();
    int getFormat() const { return m_format; }
    int getDivision() const { return m_division; }
    bool isEmpty();

    qreal currentTempo() const;
    QString getName() const { return m_lblName; }
    int songLengthTicks() const;
    void updateTempo(qreal newTempo);
    qreal ticks2millis() const { return m_ticks2millis; }
    QString currentFile() const;

signals:
    void loadingStart(int size);
    void loadingProgress(int pos);
    void loadingFinished();

public slots:
    /* SMF slots */
    void smfTrackHandler(int track);
    void smfErrorHandler(const QString& errorStr);

    /* WRK slots */
    void appendWRKmetadata(int track, long time, Sequence::TextType typ, const QByteArray &data);
    void appendWRKEvent(long ticks, MIDIEvent* ev);
    void wrkUpdateLoadProgress();
    void wrkErrorHandler(const QString& errorStr);
    void wrkFileHeader(int verh, int verl);
    void wrkEndOfFile();
    void wrkStreamEndEvent(long time);
    void wrkTrackHeader(const QByteArray& name1, const QByteArray& name2,
             int trackno, int channel, int pitch,
             int velocity, int port,
             bool selected, bool muted, bool loop);
    void wrkTimeBase(int timebase);
    void wrkGlobalVars();
    void wrkNoteEvent(int track, long time, int chan, int pitch, int vol, int dur);
    void wrkKeyPressEvent(int track, long time, int chan, int pitch, int press);
    void wrkCtlChangeEvent(int track, long time, int chan, int ctl, int value);
    void wrkPitchBendEvent(int track, long time, int chan, int value);
    void wrkProgramEvent(int track, long time, int chan, int patch);
    void wrkChanPressEvent(int track, long time, int chan, int press);
    void wrkSysexEvent(int track, long time, int bank);
    void wrkSysexEventBank(int bank, const QString& name, bool autosend, int port, const QByteArray& data);
    void wrkTextEvent(int track, long time, int typ, const QByteArray& data);
    void wrkComments(const QByteArray& cmt);
    void wrkVariableRecord(const QString& name, const QByteArray& data);
    void wrkTempoEvent(long time, int tempo);
    void wrkTrackPatch(int track, int patch);
    void wrkNewTrackHeader(const QByteArray& name,
            int trackno, int channel, int pitch,
            int velocity, int port,
            bool selected, bool muted, bool loop);
    void wrkTrackName(int trackno, const QByteArray& name);
    void wrkTrackVol(int track, int vol);
    void wrkTrackBank(int track, int bank);
    void wrkSegment(int track, long time, const QByteArray& name);
    void wrkChord(int track, long time, const QString& name, const QByteArray& data);
    void wrkExpression(int track, long time, int code, const QByteArray& text);
    void wrkTimeSignatureEvent(int bar, int num, int den);
    void wrkKeySig(int bar, int alt);
    void wrkMarker(long time, int smpte, const QByteArray& data);

private: // methods
    void sort(EventsList& list);
    void timeCalculations();
    void addMetaData(int time, int type, const QByteArray &data);
    void appendStringToList(QStringList &list, QString &s, TextType type);
    void outputEvent(MIDIEvent* ev);

private: // members
    QMap<int, EventsList> m_tracksList;
    drumstick::File::QSmf* m_smf;
    drumstick::File::QWrk* m_wrk;

    int m_returnCode;
    int m_format;
    int m_ticksDuration;
    int m_division;
    int m_pos;
    int m_curTrack;
    int m_beatMax;
    int m_barCount;
    int m_beatCount;
    int m_lowestMidiNote;
    int m_highestMidiNote;
    qreal m_tempo;
    qreal m_tempoFactor;
    qreal m_ticks2millis;
    qreal m_duration;
    qint64 m_lastBeat;
    qint64 m_beatLength;
    qint64 m_tick;
    QString m_lblName;
    QMap<int, SysExEvent*> m_savedSysexEvents;

    struct TrackMapRec {
        TrackMapRec(): channel(-1), pitch(-1), velocity(-1), port(-1), nameSet(false) { };
        int channel;
        int pitch;
        int velocity;
        int port;
        bool nameSet;
    };
    QMap<int,TrackMapRec> m_trackMap;

    struct TimeSigRec {
        int bar;
        int num;
        int den;
        long time;
    };
    QList<TimeSigRec> m_bars;

    struct TextRec {
        TextRec(QByteArray data): m_tick(0), m_track(0), m_type(TextType::None), m_text(data) { };
        TextRec(int tick, int track, TextType e, QByteArray data): m_tick(tick), m_track(track), m_type(e), m_text(data) { };
        int m_tick;
        int m_track;
        TextType m_type;
        QByteArray m_text;
    };
    QList<TextRec> m_textEvents;

    QString m_currentFile;
    QString m_fileFormat;
    bool m_timeSignatureSet;
    bool m_keySignatureSet;
    bool m_copyrightSet;
};

#endif // SEQUENCE_H
