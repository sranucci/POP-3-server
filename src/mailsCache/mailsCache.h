#ifndef MAIL_CACHE
#define MAIL_CACHE
#define MAXFILENAME 256

typedef enum executionStatus {
    FINISHED,
    NOT_FINISHED,
    FAILED,
} executionStatus;

typedef struct mailCache mailCache;

typedef struct mailInfo {
    char filename[MAXFILENAME];
    int mailNo;
    long sizeInBytes;
} mailInfo;

mailCache * initCache(char * username);
void freeCache(mailCache * mailCache);
int openMail(mailCache * mailCache, int mailNo);
executionStatus getNCharsFromMail(mailCache * mailCache, int * characters, char * buffer);
int closeMail(mailCache * mailCache);
int toBegin(mailCache * mailCache);
int hasNext(mailCache * mailCache);
int next(mailCache * mailCache, mailInfo * mailInfo);
int getMailInfo(mailCache * mailCache, int mailNo, mailInfo * mailInfo);
int markMailToDelete(mailCache * mailCache, int mailNo);
int resetToDelete(mailCache * mailCache);
int deleteMarkedMails(mailCache * mailCache);
int getAmountOfMails(mailCache * mailCache); //excludes mails marked to be deleted
long long getSizeOfMails(mailCache * mailCache); //excludes mails marked to be deleted

#endif