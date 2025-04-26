#include "fileManager.h"


#include"AmebaFatFS.h"

AmebaFatFS fs;
String rootPath="";

void fsBegin(){
  fs.begin();
  delay(1000); // give some time to initialized
  rootPath = String(fs.getRootPath());
 }
bool fileRemove( String fname){
  if (fs.status()){    
   return fs.remove(rootPath + fname);
  }
    
  
}
int fileRead(String fname, unsigned char* buff, int size){
  int len =0;
  if (fs.status()){    
    File file = fs.open(rootPath + fname);
    if( file != NULL){
      len = file.read((void*)buff, size); 
      file.close(); 
    }
  }
  return len;
}

int fileWrite( String fname, unsigned char* buff, int size){
  int len =0;

  if (fs.status()){   
    File file = fs.open(rootPath + fname);
    if( file != NULL){
      len = file.write(buff, size);  
      file.close();
    }
  }
  return len;
}



String getRootPath(){  
  return String(fs.getRootPath());
}

// return the read buffer size
int getDirList(char *rxList, int size){
  char *p =0;
  char *q =0;
  /* list root directory and put results in buf */
  memset(rxList, 0, size);
  fs.readDir(fs.getRootPath(), rxList, size);

  Serial.print("Files under");
  Serial.print( fs.getRootPath());
  Serial.println(":\r\n");  
  /* the filenames are separated with '\0', so we scan one by one */
  
  p = rxList;
  q=p;
  int strLen = strlen(p);
  while ( strLen> 0) {
    Serial.println(p);
    p += strlen(p) + 1;
    strLen = strlen(p);
  }
  Serial.print("buffer len of dir list =  ");
  Serial.println(p-q);    
  return p-q; // return the length of the read buffer
}


