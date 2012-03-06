#ifndef CLIP_H
#define CLIP_H

#include <QtSql>



/**
 * @class cobraClip clip.h "clip."
 *
 * The cobraClip handles all underlying sqllite and obfuscates it.
 * This will allow functions to be called and just return file titles and
 * fields.  You can then poopulate the file lists with this information.
 */
class cobraClip : public QObject {
    Q_OBJECT

public:
    cobraClip(QObject* parent = NULL );
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
     * @fn void addToDatabase(struct fileInfo)
     * addToDatabase will take the extra information as well as the location of the file to the database.
     * @param fileInfo is a struct with the information that will be populated in the database.
     */
    void addToDatabase(struct fileInfo);



public slots:

signals:

protected:

    /**
     * @fn void connectToDatabase(QStrin databaseName)
     * connect to the database.
     * @param desc databaseName is the sqllite database to connect to.
     */
    void connetToDatabase(QString databaseName);
   

    QString                             m_sDataBaseLocation;
    QString                             m_sTitle;
    QString                             m_sHash;
    QString                             m_sPath;
    QString                             m_sTags;
    QString                             m_sModifiedTime;
    int                                 m_iSize;
    int                                 m_iUID;
    
};

class cobraClipList : public cobraClip {
   Q_OBJECT

public:
   cobraClipList(QObject* parent = NULL);
   virtual ~cobraClipList();

   void                         enumClips(QVector<int>& );
   cobraClip                    getClip(int uid);
   bool                         updateClip(cobraClip& clip);
   bool                         removeClip(int uid);
   bool                         addClip(cobraClip& clip);
};

#endif // clip_H


