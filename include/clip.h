#ifndef CLIP_H
#define CLIP_H

#include <QSqlDatabase>
#include <QtSql>



/**
 * @class cobraClip clip.h "clip."
 *
 * The cobraClip handles all underlying sqllite and abstracts it.
 * This will allow functions to be called and just return file titles and
 * fields.  You can then poopulate the file lists with this information.
 */
class cobraClip {

public:
    cobraClip();
    virtual ~cobraClip();

    /**
     * @fn QString getTitle() 
     * @return the title of a Clip
     */
    QString getTitle() {
        return (m_sTitle);
    }

    /**
     * @fn QString getHash() 
     * @return the hash of a Clip
     */
    QString getHash() {
        return(m_sHash);
    }

    /**
     * @fn QString getPath() 
     * @return the path of a Clip
     */
    QString getPath() {
        return(m_sPath);
    }

    /**
     * @fn QString getTags() 
     * @return the tag of a Clip
     */
    QString getTags() {
        return(m_sTags);
    }

    /**
     * @fn QString getModifiedTime() 
     * @return the modification time of a Clip
     */
    QString getModifiedTime() {
        return(m_sModifiedTime);
    }

    /**
     * @fn QString getDescription()
     * @return the description of a Clip
     */
    QString getDescription() {
        return(m_sDescription);
    }

    /**
     * @fn int getSize() 
     * @return the size of a Clip
     */
    int getSize() {
        return(m_iSize);
    }

    /**
     * @fn int getUID() 
     * @return the unique id of a Clip
     */
    int getUID() {
        return(m_iUID);
    }

    /**
     * @fn QString setTitle() 
     * sets the title of a Clip
     */
    void setTitle(QString title) {
        m_sTitle = title;
    }

    /**
     * @fn void setHash() 
     * sets the hash of a Clip
     */
    void setHash(QString hash) {
        m_sHash = hash;
    }

    /**
     * @fn void setPath() 
     * sets the path of a Clip
     */
    void setPath(QString path) {
        m_sPath = path;
    }

    /**
     * @fn void setTags() 
     * sets the tag of a Clip
     */
    void setTags(QString tags) {
        m_sTags = tags;
    }

    /**
     * @fn void setModifiedTime() 
     * sets the modification time of a Clip
     */
    void setModifiedTime(QString time) {
        m_sModifiedTime = time;
    }

    /**
     * @fn void setDescription()
     * sets the description of a Clip
     */
    void setDescription(QString desc) {
        m_sDescription = desc;
    }

    /**
     * @fn void setSize() 
     * sets the size of a Clip
     */
    void setSize(int size) {
        m_iSize = size;
    }

    /**
     * @fn void setUID() 
     * sets the unique id of a Clip
     */
    void setUID(int uid) {
        m_iUID = uid;
    }



//public slots:

//signals:

protected:

    QString                             m_sTitle;
    QString                             m_sHash;
    QString                             m_sPath;
    QString                             m_sTags;
    QString                             m_sModifiedTime;
    QString                             m_sDescription;
    int                                 m_iSize;
    int                                 m_iUID;
    
};

class cobraClipList {

public:
    cobraClipList(QString dbName = ":memory:");
    virtual ~cobraClipList();

   void                         enumClips(QVector<int>& );
   cobraClip                    getClip(int uid);
   virtual bool                 updateClip(cobraClip& clip);
   virtual bool                 removeClip(int uid);
   virtual bool                 addClip(cobraClip& clip);

protected:
   bool                         sqlQuery(QString&);

protected:
   QString                      m_sDBName;
   QSqlDatabase                 m_dbDatabase;
};

#endif // clip_H


