#ifndef TIMELINE_H
#define TIMELINE_H

#include <QSqlDatabase>
#include <QtSql>
#include <QDateTime>
#include <debug.h>


/**
 * @class cobraTimeline timeline.h "timeline."
 *
 * The cobraTimeline handles all underlying sqllite and abstracts it.
 * This will allow functions to be called and just return file titles and
 * fields.  You can then poopulate the file lists with this information.
 */
class cobraTimeline {

public:
    cobraTimeline();
    virtual ~cobraTimeline();

    /**
     * @fn QString getTitle() 
     * @return the title of a Timeline
     */
    QString getTitle() {
        return (m_sTitle);
    }

    /**
     * @fn QString getStart() 
     * @return the start time of a Timeline
     */
    QDateTime getStartTime() {
        return(m_dtStart);
    }

    /**
     * @fn QString getDescription()
     * @return the description of a Timeline
     */
    QString getDescription() {
        return(m_sDescription);
    }

    /**
     * @fn int getUid()
     * @return the unique id of a Timeline
     */
    int getUid() {
        return(m_iUid);
    }

    /**
     * @fn QString setTitle() 
     * sets the title of a Timeline
     */
    void setTitle(QString title) {
        m_sTitle = title;
    }

    /**
     * @fn void setStart() 
     * sets the start time of a Timeline
     */
    void setStart(QVariant start) {
        //m_dtStart = QDateTime(start,"'M'M'd'd'y'yyhh:mm:ss");
        m_dtStart = start.toDateTime();
        debug(LOW, "Setting start time to %s for timeline %i \n", 
                    qPrintable(m_dtStart.toString("yyyy-MM-dd HH:mm:ss")),
                    m_iUid);
    }

    void setStartTime() {
        m_dtStart = QDateTime::currentDateTime();
        debug(LOW, "Setting start time to %s for timeline %i \n", 
                    qPrintable(m_dtStart.toString("yyyy-MM-dd HH:mm:ss")),
                    m_iUid);
    }

    /**
     * @fn void setDescription()
     * sets the description of a Timeline
     */
    void setDescription(QString desc) {
        m_sDescription = desc;
    }

    /**
     * @fn void setUid()
     * sets the unique id of a Timeline
     */
    void setUid(int Uid) {
        m_iUid = Uid;
    }

//public slots:

//signals:

protected:

    QString                  m_sTitle;
    QDateTime                m_dtStart;
    QString                  m_sDescription;
    int                      m_iUid;

};


/**
 * @class cobraMark mark.h "mark."
 *
 * The cobraMark handles all underlying sqllite and abstracts it.
 * This will allow functions to be called and just return file titles and
 * fields.  You can then poopulate the file lists with this information.
 */
class cobraMark {

public:
    cobraMark();
    cobraMark(cobraTimeline& timeline);
    virtual ~cobraMark();

    /**
     * @fn int getTimeOffset() 
     * @return the offset of a Mark
     */
    int getTimeOffset() {
        return (m_iTimeOffset);
    }

    /**
     * @fn QString getComment() 
     * @return the commenton a Mark
     */
    QString getComment() {
        return(m_sComment);
    }

    /**
     * @fn int getTimelineKey()
     * @return the Timeline key of a Mark
     */
    int getTimelineKey() {
        return(m_iTimelineKey);
    }

    /**
     * @fn int getUid()
     * @return the unique id of a Mark
     */
    int getUid() {
        return(m_iUid);
    }

    /**
     * @fn int setTimeOffset() 
     * sets the offset of a Mark
     */
    void setTimeOffset(const QDateTime& startTime) {
        debug(LOW,"start time : Current Time - %s : %s\n", 
               qPrintable(startTime.toString()), 
               qPrintable(QDateTime::currentDateTime().toString()));
        m_iTimeOffset = startTime.secsTo(QDateTime::currentDateTime());
    }

    void setTimeOffset(int offset) {
        m_iTimeOffset = offset;
    }

    /**
     * @fn void setComment() 
     * sets the hash of a Mark
     */
    void setComment(QString comment) {
        m_sComment = comment;
    }

    /**
     * @fn void setTimelineKey()
     * sets the Timeline key of a Mark
     */
    void setTimelineKey(int key) {
        m_iTimelineKey = key;
    }

    /**
     * @fn void setUid()
     * sets the unique id of a Mark
     */
    void setUid(int Uid) {
        m_iUid = Uid;
    }

//public slots:

//signals:

protected:

    QString                  m_sComment;
    int                      m_iTimeOffset;
    int                      m_iTimelineKey;
    int                      m_iUid;
    
};

class cobraTimelineList {
public:
    cobraTimelineList(QString dbName = ":memory:");
    virtual ~cobraTimelineList();
 
    void                     setName(QString dbName);
 
    void                     enumTimelines(QVector<int>& vector);
    void                     enumTimelineMarkers(QVector<int>& vector,
                                                  cobraTimeline& timeline);
    cobraTimeline            getTimeline(int Uid);
    cobraMark                getMark(int Uid);
    virtual bool             updateTimeline(cobraTimeline& timeline);
    virtual bool             updateMark(cobraMark& mark);
    virtual bool             removeTimeline(int Uid);
    virtual bool             removeMark(int Uid);
    virtual bool             addTimeline(cobraTimeline& timeline);
    virtual bool             addMark(cobraMark& mark);
 
    virtual bool             contains(int Uid);
    bool                     isValid() const;

protected:
    bool                     sqlQuery(QString&);
    static int               nextList();

protected:
    QString                  m_sDBName;
    QSqlDatabase             m_dbDatabase;

    static int               m_iLists;
};

#endif // timeline_H


