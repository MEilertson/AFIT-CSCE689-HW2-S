#include <argon2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <list>
#include <chrono>
#include "PasswdMgr.h"
#include "FileDesc.h"
#include "strfuncts.h"

const int hashlen = 32;
const int saltlen = 16;

PasswdMgr::PasswdMgr(const char *pwd_file):_pwd_file(pwd_file) {

}


PasswdMgr::~PasswdMgr() {

}

/*******************************************************************************************
 * checkUser - Checks the password file to see if the given user is listed
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkUser(const char *name) {
   std::vector<uint8_t> passwd, salt;

   bool result = findUser(name, passwd, salt);

   return result;
     
}

/*******************************************************************************************
 * checkPasswd - Checks the password for a given user to see if it matches the password
 *               in the passwd file
 *
 *    Params:  name - username string to check (case insensitive)
 *             passwd - password string to hash and compare (case sensitive)
 *    
 *    Returns: true if correct password was given, false otherwise
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkPasswd(const char *name, const char *passwd) {
   std::vector<uint8_t> userhash(hashlen); // hash from the password file
   std::vector<uint8_t> passhash(hashlen); // hash derived from the parameter passwd
   std::vector<uint8_t> salt(saltlen);

   // Check if the user exists and get the passwd string
   if (!findUser(name, userhash, salt))
      return false;

   hashArgon2(passhash, salt, passwd, &salt);

   if (userhash == passhash)
      return true;

   return false;
}

/*******************************************************************************************
 * changePasswd - Changes the password for the given user to the password string given
 *
 *    Params:  name - username string to change (case insensitive)
 *             passwd - the new password (case sensitive)
 *
 *    Returns: true if successful, false if the user was not found
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            writing
 *
 *******************************************************************************************/

bool PasswdMgr::changePasswd(const char *name, const char *passwd) {

   // Insert your insane code here

   return true;
}

/*****************************************************************************************************
 * readUser - Taking in an opened File Descriptor of the password file, reads in a user entry and
 *            loads the passed in variables
 *
 *    Params:  pwfile - FileDesc of password file already opened for reading
 *             name - std string to store the name read in
 *             hash, salt - vectors to store the read-in hash and salt respectively
 *
 *    Returns: true if a new entry was read, false if eof reached 
 * 
 *    Throws: pwfile_error exception if the file appeared corrupted
 *
 *****************************************************************************************************/

bool PasswdMgr::readUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   // Insert your perfect code here!

   //Getting username
   int returnStatus = pwfile.readStr(name);
   if(returnStatus < 0) //somethings wrong
   {
      throw(pwfile_error("Error reading username from password file"));
   } 
   else if (returnStatus == 0) //end of file reached
   {
         return false;
   }

   //Once username is read, expecting 32 byte hash
   returnStatus = pwfile.readBytes<uint8_t>(hash, hashlen);
   if(returnStatus == -1)
   {
      throw(pwfile_error("Hash Read Error"));
   } 
   else if (returnStatus == -2) //Hash not long enough.. corrupted?
   {
      throw(pwfile_error("Hash corrupted"));
   }
   else
   {
      //successful hash read
   }

   //Once hash is read in, expecting 16 byte salt
   returnStatus = pwfile.readBytes<uint8_t>(salt, saltlen);
   if(returnStatus == -1)
   {
      throw(pwfile_error("Salt Read Error"));
   } 
   else if (returnStatus == -2) //Salt not long enough.. corrupted?
   {
      throw(pwfile_error("Salt corrupted"));
   }
   else
   {
      std::string endEntryNewLine;
      pwfile.readStr(endEntryNewLine);
      //successful Salt read
   }
   return true;
}

/*****************************************************************************************************
 * writeUser - Taking in an opened File Descriptor of the password file, writes a user entry to disk
 *
 *    Params:  pwfile - FileDesc of password file already opened for writing
 *             name - std string of the name 
 *             hash, salt - vectors of the hash and salt to write to disk
 *
 *    Returns: bytes written
 *
 *    Throws: pwfile_error exception if the writes fail
 *
 *****************************************************************************************************/

int PasswdMgr::writeUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   int results = 0;
   if( (results = pwfile.writeFD(name.append("\n"))) < 0 ){
      throw(pwfile_error("Error writing name to pwdfile"));
   }
   results += pwfile.writeBytes<uint8_t>(hash);
   results += pwfile.writeBytes<uint8_t>(salt);
   results += pwfile.writeFD("\n");
   return results; 
}

/*****************************************************************************************************
 * findUser - Reads in the password file, finding the user (if they exist) and populating the two
 *            passed in vectors with their hash and salt
 *
 *    Params:  name - the username to search for
 *             hash - vector to store the user's password hash
 *             salt - vector to store the user's salt string
 *
 *    Returns: true if found, false if not
 *
 *    Throws: pwfile_error exception if the pwfile could not be opened for reading
 *
 *****************************************************************************************************/

bool PasswdMgr::findUser(const char *name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt) {

   FileFD pwfile(_pwd_file.c_str());

   // You may need to change this code for your specific implementation

   if (!pwfile.openFile(FileFD::readfd))
      throw pwfile_error("Could not open passwd file for reading");

   // Password file should be in the format username\n{32 byte hash}{16 byte salt}\n
   bool eof = false;
   while (!eof) {
      std::string uname;

      if (!readUser(pwfile, uname, hash, salt)) {
         eof = true;
         continue;
      }

      if (!uname.compare(name)) {
         pwfile.closeFD();
         return true;
      }
   }

   hash.clear();
   salt.clear();
   pwfile.closeFD();
   return false;
}


/*****************************************************************************************************
 * hashArgon2 - Performs a hash on the password using the Argon2 library. Implementation algorithm
 *              taken from the http://github.com/P-H-C/phc-winner-argon2 example. 
 *
 *    Params:  dest - the std string object to store the hash
 *             passwd - the password to be hashed
 *
 *    Throws: runtime_error if the salt passed in is not the right size
 *****************************************************************************************************/
void PasswdMgr::hashArgon2(std::vector<uint8_t> &ret_hash, std::vector<uint8_t> &ret_salt, 
                           const char *in_passwd, std::vector<uint8_t> *in_salt /*=NULL*/) {
   // Hash those passwords!!!!
   if(in_salt != NULL){
      
   }

   //used from https://github.com/P-H-C/phc-winner-argon2
   uint32_t t_cost = 2;            // 1-pass computation
   uint32_t m_cost = (1<<16);      // 64 mebibytes memory usage
   uint32_t parallelism = 1;       // number of threads and lanes

   uint8_t hashArray[hashlen];

   argon2i_hash_raw(t_cost, m_cost, parallelism, (uint8_t *)in_passwd, strlen((char *)in_passwd), &ret_salt[0], saltlen, &ret_hash[0], hashlen);
   //memcpy((void *)&ret_hash, hashArray, 32);
}

/****************************************************************************************************
 * addUser - First, confirms the user doesn't exist. If not found, then adds the new user with a new
 *           password and salt
 *
 *    Throws: pwfile_error if issues editing the password file
 ****************************************************************************************************/

void PasswdMgr::addUser(const char *name, const char *passwd) {
   // Add those users!
   FileFD pwfile(_pwd_file.c_str());
   if(checkUser(name))
      return;


   std::vector<uint8_t> ret_hash(hashlen);
   //std::vector<uint8_t> in_salt(saltlen);
   std::vector<uint8_t> ret_salt(saltlen);
   if (!pwfile.openFile(FileFD::appendfd))
      throw pwfile_error("Could not open passwd file for adding user");
   int result;

   genSalt(ret_salt);

   hashArgon2(ret_hash, ret_salt, passwd, &ret_salt);

   std::string username = name;
   writeUser(pwfile, username, ret_hash, ret_salt);
}

void PasswdMgr::genSalt(std::vector<uint8_t> &salt) {
   uint8_t byte;
   if (salt.size() != saltlen)
      throw(pwfile_error("Improper salt vector size passed into genSalt"));
   for (size_t i = 0; i < saltlen; i++)
   {
      byte = rand() % 16;
      salt[i] = byte;
   }
   
}

