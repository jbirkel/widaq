// wdqapiJNI.cpp : Defines the exported functions for the DLL application.
//

//#include "stdafx.h"

#include <jni.h>
#include <stdio.h>
#include "wdqapiJNI.h"
#include "wdqapi.h"

#include "jhbCommon.h"

static void __stdcall _dbgOut( const wchar_t *psz );
static void __stdcall _dbgOut( const char    *psz );

static void _jniOut( const char *psz ) { _dbgOut( psz ); }

PrintProxy<char> _ppJNI( _jniOut );

//#define _WDQAPICLASS      "Lwdqapi/WdqApi"
//#define  WDQAPICLASS_ROOT _WDQAPICLASS ";"
//#define  WDQAPICLASS(cls) _WDQAPICLASS "$" #cls ";"
#define _WDQAPICLASS      "wdqapi/WdqApi"
#define  WDQAPICLASS_ROOT _WDQAPICLASS 
#define  WDQAPICLASS(cls) _WDQAPICLASS "$" #cls 

// --------------------------------------------------------------------------------------
//
// Helpers (Private) 
//
// --------------------------------------------------------------------------------------
#define NELEM(a) (sizeof(a)/sizeof((a)[0]))

typedef CvtStr<wdq_Char_t> CvtStrWdq;

// Sets a fixed-length wdq_Char_t buffer from a JNI object. 
static void _setWdqFromString( wdq_Char_t *szWdq, int size, JNIEnv *env, jobject S, jfieldID fid ) { 
   jstring js = (jstring)env->GetObjectField(S, fid);   
   const char *sz = env->GetStringUTFChars(js, 0); 
   //_ppJNI.printf( "{JNI}: _setWdqStr: %s\n", sz );
   
   memset( szWdq, 0, size * sizeof szWdq[0] );  
   _wdqStrNCpy( szWdq, CvtStrWdq( sz ), size-1 );   
   env->ReleaseStringUTFChars(js, sz);       
}

static void _setStringFromWdq( JNIEnv *env, jobject S, jfieldID fid, wdq_Char_t *szWdq ) { 
   jstring str = env->NewStringUTF( CvtStrA( szWdq ));
   env->SetObjectField( S, fid, str );
} 

static void _setStringFromAsc( JNIEnv *env, jobject S, jfieldID fid, wdq_Ascii_t *szAsc ) {
   jstring str = env->NewStringUTF( szAsc );
   env->SetObjectField( S, fid, str );
}   
   
// Sets a fixed-length wdq_Ascii_t buffer from a JNI object.
static void _setAscFromString( char *szAsc, int size, JNIEnv *env, jobject S, jfieldID fid ) {    
   jstring js = (jstring)env->GetObjectField(S, fid);   
   const char *sz = env->GetStringUTFChars(js, 0); 
   //_ppJNI.printf( "{JNI}: _setAscStr: %s\n", sz );
   
   memset ( szAsc, 0 , size   );  
   strncpy( szAsc, sz, size-1 );   
   env->ReleaseStringUTFChars(js, sz);   
}

// Sets a Java byte array corresponding to a MAC address
static void _setObjectFromMac( wdq_Byte_t *MAC, JNIEnv *env, jobject S, jfieldID fid ) {
   jbyteArray a = (jbyteArray)env->GetObjectField(S,fid);
   env->SetByteArrayRegion( a, 0, WDQ_MAC_ADDR_SIZE, (jbyte*)&MAC[0] );   
}   

/*
// Sets a C byte array from a Java byte array corresponding to a MAC address
static void _setMacFromObject( wdq_Byte_t *MAC, JNIEnv *env, jobject S, jfieldID fid ) {
   jbyteArray a = (jbyteArray)env->GetObjectField(S,fid);
   env->GetByteArrayRegion( a, 0, WDQ_MAC_ADDR_SIZE, (jbyte*)&MAC[0] );   
} 
*/

// --------------------------------------------------------------------------------------
// Helps with accessing wdq_SSID_t structures.

class ParseSSID {

   static jfieldID fid_len  ; // Field IDs
   static jfieldID fid_name ;
   
public:   

   // Init field IDs (need to do once/safe to do many) 
   static void getFids( JNIEnv *env ) {
      if (NULL != fid_len) return;
      jclass  cls = env->FindClass( WDQAPICLASS(SSID_t) );  
      fid_len  = env->GetFieldID( cls, "len" , "I"  );
      fid_name = env->GetFieldID( cls, "name", "[B" );      
   }   
   
   // Sets a JNI object from a WDQAPI SSID structure.
   static bool setObjectFromSsid( wdq_SSID_t &ssid, JNIEnv *env, jobject S, jfieldID fid ) {
      if (NULL == fid_len) return false;   
      jobject jobj = env->GetObjectField( S, fid );   
      env->SetIntField( jobj, fid_len, ssid.len );
      jbyteArray jba = (jbyteArray)env->GetObjectField( jobj, fid_name );
      env->SetByteArrayRegion( jba, 0, min(ssid.len,NELEM(ssid.name)), (jbyte*)&ssid.name[0] );      
      return true;            
   } 
   
   // Sets a JNI object from a WDQAPI SSID structure.
   static bool setSsidFromObject( wdq_SSID_t &ssid, JNIEnv *env, jobject S, jfieldID fid ) {
      if (NULL == fid_len) return false;   
      jobject jobj = env->GetObjectField( S, fid );   
      ssid.len = env->GetIntField( jobj, fid_len );

      memset ( ssid.name, 0, sizeof ssid.name   );        
      jbyteArray jba = (jbyteArray)env->GetObjectField( jobj, fid_name );
      env->GetByteArrayRegion( jba, 0, min(ssid.len,NELEM(ssid.name)), (jbyte*)&ssid.name[0] );
      return true;   
   }
};

jfieldID ParseSSID::fid_len  = NULL ; 
jfieldID ParseSSID::fid_name ;

// --------------------------------------------------------------------------------------
// Class:     WdqApi
// Method:    NativeInit
// Signature: ()V
// 
// NOTE: Not part of the WDQAPI.  Used to initialize the native side of the JNI layer.
// --------------------------------------------------------------------------------------

JavaVM *_JVM;
jclass  _CLS;

JNIEXPORT void JNICALL Java_wdqapi_WdqApi_NativeInit(JNIEnv *env, jclass) {
   //int err = env->GetJavaVM( &_JVM );
   //if (0 != err) {
   //   printf( "Java_wdqapi_WdqApi_NativeInit: GetJavaVM returned %d\n", err );
   //}   
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void * /*reserved*/) {
   printf( "JNI_OnLoad\n" );
   
   _JVM = jvm;  // cache the JavaVM pointer 
   
   JNIEnv *env;
   if (jvm->GetEnv((void **)&env, JNI_VERSION_1_2)) {
      return JNI_ERR; /* JNI version not supported */
   }

   // Create weak global reference to WdqApi class   
   _CLS = (jclass) env->NewWeakGlobalRef( env->FindClass( WDQAPICLASS_ROOT ) );
   
   return JNI_VERSION_1_2;   
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void * /*reserved*/)  {
   printf( "JNI_OnUnload\n" );
   
   JNIEnv *env;
   if (jvm->GetEnv( (void **)&env, JNI_VERSION_1_2)) {
       return;
   }
   
   env->DeleteWeakGlobalRef( _CLS );
}

// --------------------------------------------------------------------------------------
//
// JNI API Functions (Public)
//
// --------------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqGetVersion(JNIEnv *, jobject, jint reserved ) {
   return WdqGetVersion(reserved);
}

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqEnableAutoNetworkSwitch(JNIEnv *, jobject, jboolean bEnable, jint reserved) {
   return WdqEnableAutoNetworkSwitch( bEnable, reserved );
}

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqStartScan(JNIEnv *, jobject, jint reserved) {
   return WdqStartScan( reserved ); 
}

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqSetCurrentNetwork(JNIEnv *env, jobject, jstring name, jint flags) {
  const char *str = env->GetStringUTFChars(name, 0);
  WDQAPIERR err = WdqSetCurrentNetwork( CvtStrWdq(str), (wdq_apiFlags_e)flags );
  env->ReleaseStringUTFChars(name, str);   
  return err;
}

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqDeleteNetwork(JNIEnv *env, jobject, jstring name, jint flags) {
  const char *str = env->GetStringUTFChars(name, 0);
  WDQAPIERR err = WdqDeleteNetwork( CvtStrWdq(str), (wdq_apiFlags_e) flags );
  env->ReleaseStringUTFChars(name, str);   
  return err;
}

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqSetCertPath(JNIEnv *env, jobject, jstring certPath, jint flags) {
  const char *str = env->GetStringUTFChars(certPath, 0);
  WDQAPIERR err = WdqSetCertPath( CvtStrWdq(str), (wdq_apiFlags_e)flags );
  env->ReleaseStringUTFChars(certPath, str);   
  return err;
}


// --------------------------------------------------------------------------------------
/*
 * Class:     WdqApi
 * Method:    WdqSetDbgOut
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)V
 */
  
jmethodID _midDbgOut = NULL;

static void __stdcall _dbgOut( const wchar_t *psz ) {
   //JNIEnv *env=0; _JVM->AttachCurrentThread((void **)&env, NULL);   
   JNIEnv *env=0; _JVM->AttachCurrentThread(&env, NULL);   
   if (env) {
      jstring str = env->NewStringUTF( CvtStrA( psz ));
      env->CallStaticVoidMethod( _CLS, _midDbgOut, str );      
   }  
}
static void __stdcall _dbgOut( const char *psz ) {
   JNIEnv *env=0; _JVM->AttachCurrentThread((void **)&env, NULL);   
   if (env) {
      jstring str = env->NewStringUTF( psz );
      env->CallStaticVoidMethod( _CLS, _midDbgOut, str );      
   }  
}
JNIEXPORT void JNICALL Java_wdqapi_WdqApi_WdqSetDbgOut(JNIEnv *env, jobject obj, jstring cbMethod, jint ) {
   
   if (NULL == cbMethod) {
      WdqSetDbgOut( 0, 0 );
      _midDbgOut = NULL;
   }   
   else {
      const char *sFn = env->GetStringUTFChars( cbMethod, 0);      
      jmethodID mid = env->GetStaticMethodID( _CLS, sFn, "(Ljava/lang/String;)V");
      env->ReleaseStringUTFChars( cbMethod, sFn );               
      
      if (mid == 0) return;
      _midDbgOut = mid;
      WdqSetDbgOut( _dbgOut, 0 );   
   }  
}

// --------------------------------------------------------------------------------------  

 /*
 * Class:     WdqApi
 * Method:    WdqStartPingTest
 * Signature: (Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;I)I
 */

bool bInit_pingResults_fids = true;

jfieldID fid_pingResults_status   ;
jfieldID fid_pingResults_error    ;
jfieldID fid_pingResults_lost     ;
jfieldID fid_pingResults_recv     ;
jfieldID fid_pingResults_timeLast ;
jfieldID fid_pingResults_timeTotal;
jfieldID fid_pingResults_timeMin  ;
jfieldID fid_pingResults_timeMax  ;

//jclass   g_clsPingRes;

jmethodID _midPingRes ;
jmethodID _midPingEnum;
jmethodID _midPingCb  ;


std::string _sPingFunc;

static void __stdcall _pingCallback( wdq_pingResults_t *_S, UINT /*reserved*/ ) {

   JNIEnv *env;
   int err = _JVM->AttachCurrentThread((void **)&env, NULL);

   if (!env) {
      _ppJNI.printf( "_pingCallback: AttachCurrentThread returned %d, env=%d\n", err, env );
      return;
   }
   
   jclass cls = env->FindClass( WDQAPICLASS(pingResults_t));
   
   if(bInit_pingResults_fids) {
      bInit_pingResults_fids = false;
      
      fid_pingResults_status    = env->GetFieldID( cls, "status"   , WDQAPICLASS(pingResultsStatus_e) );        
      fid_pingResults_error     = env->GetFieldID( cls, "error"    , "I" );
      fid_pingResults_lost      = env->GetFieldID( cls, "lost"     , "I" );
      fid_pingResults_recv      = env->GetFieldID( cls, "recv"     , "I" );
      fid_pingResults_timeLast  = env->GetFieldID( cls, "timeLast" , "I" );
      fid_pingResults_timeTotal = env->GetFieldID( cls, "timeTotal", "I" );
      fid_pingResults_timeMin   = env->GetFieldID( cls, "timeMin"  , "I" );
      fid_pingResults_timeMax   = env->GetFieldID( cls, "timeMax"  , "I" );
      
      _midPingRes  = env->GetMethodID      ( cls , "pingResults_t"   , "()V" );   
      _midPingEnum = env->GetStaticMethodID( cls , "IntToEnum"       , "(I)" WDQAPICLASS(pingResultsStatus_e) );            
      _midPingCb   = env->GetStaticMethodID( _CLS, _sPingFunc.c_str(), "(" WDQAPICLASS(pingResults_t) ")V" );            
   }         
   
   jobject S = env->NewObject( cls, _midPingRes );
   
   jobject Enum = env->CallStaticObjectMethod( cls, _midPingEnum, _S->status );   
   
   env->SetObjectField( S, fid_pingResults_status   , Enum          );
   env->SetIntField   ( S, fid_pingResults_error    , _S->error     );
   env->SetIntField   ( S, fid_pingResults_lost     , _S->lost      );
   env->SetIntField   ( S, fid_pingResults_recv     , _S->recv      );
   env->SetIntField   ( S, fid_pingResults_timeLast , _S->timeLast  );
   env->SetIntField   ( S, fid_pingResults_timeTotal, _S->timeTotal );
   env->SetIntField   ( S, fid_pingResults_timeMin  , _S->timeMin   );
   env->SetIntField   ( S, fid_pingResults_timeMax  , _S->timeMax   );
   
   env->DeleteLocalRef( Enum );        
      
   // This also works!
   //jmethodID mid = env->GetMethodID( g_clsPingRes, "postJNI", "()V" );      
   //env->CallVoidMethod( S, mid );    
   
   env->CallStaticVoidMethod( _CLS, _midPingCb, S ); 
   
   env->DeleteLocalRef( S );     
   
   if (_S->status == WDQ_PINGRES_FINISHED) {
      _JVM->DetachCurrentThread();   
      bInit_pingResults_fids = true;
   } 
}
                      
JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqStartPingTest(JNIEnv *env, jobject, jstring ipAddr, jint count, jstring cbMethod, jint reserved) {
   
   {  const char *sz = env->GetStringUTFChars( cbMethod, 0); 
      _sPingFunc = sz;
      env->ReleaseStringUTFChars( cbMethod, sz );       
   }   

   const char *s = env->GetStringUTFChars( ipAddr, 0);            
   int err = WdqStartPingTest( CvtStrWdq( s ), count, _pingCallback, reserved ); 
   env->ReleaseStringUTFChars( ipAddr, s );             
   
   return err;
}
  
JNIEXPORT void JNICALL Java_wdqapi_WdqApi_WdqStopPingTest(JNIEnv *, jobject, jint reserved) {
   WdqStopPingTest( reserved ); 
}
  
// --------------------------------------------------------------------------------------  

 /*
 * Class:     WdqApi
 * Method:    WdqGetStatus
 * Signature: (Lwdqapi/WdqApi/wdq_status_t;I)I
 */
 
 bool bInit_status_fids = true;
 
jfieldID fid_status_verDriver     ; 
jfieldID fid_status_verSupplicant ;
jfieldID fid_status_adapterType   ; 
jfieldID fid_status_regDomain     ;
jfieldID fid_status_MAC           ;
jfieldID fid_status_MACAP         ;
jfieldID fid_status_IPaddr        ;
jfieldID fid_status_IPaddrAP      ;	
jfieldID fid_status_IPaddrRCServer;
jfieldID fid_status_SSID          ;
jfieldID fid_status_connStatus    ;	
jfieldID fid_status_channelNum    ;   
jfieldID fid_status_RSSI          ;  
jfieldID fid_status_SNR           ;
jfieldID fid_status_bitRate       ;
jfieldID fid_status_framesTx      ;
jfieldID fid_status_framesRx      ;
jfieldID fid_status_bytesTx       ;
jfieldID fid_status_bytesRx       ;	
jfieldID fid_status_adapterReady  ;
jfieldID fid_status_probing       ;  

//fids_SSID_t fids_SSID_status ;
                               
JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqGetStatus(JNIEnv *env, jobject, jobject S, jint reserved) {

   wdq_status_t _S;
   int err = WdqGetStatus( &_S, reserved );
   if (WDQ_APIERR_SUCCESS != err) {
      _ppJNI.printf("***ERROR: WdqGetStatus returned %d\n", err );
      return err;
   }
   
   if(bInit_status_fids) {
      bInit_status_fids = false;
      
      jclass cls = env->FindClass( WDQAPICLASS(status_t));    
      fid_status_verDriver      = env->GetFieldID(cls, "_I_verDriver"    , "I"  );
      fid_status_verSupplicant  = env->GetFieldID(cls, "_I_verSupplicant", "I"  );
      fid_status_adapterType    = env->GetFieldID(cls, "_I_adapterType"  , "I"  );
      fid_status_regDomain      = env->GetFieldID(cls, "_I_regDomain"    , "I"  );
      fid_status_MAC            = env->GetFieldID(cls, "MAC"             , "[B" );
      fid_status_MACAP          = env->GetFieldID(cls, "MACAP"           , "[B" );        
      fid_status_IPaddr         = env->GetFieldID(cls, "IPaddr"          , "I"  );
      fid_status_IPaddrAP       = env->GetFieldID(cls, "IPaddrAP"        , "I"  );
      fid_status_IPaddrRCServer = env->GetFieldID(cls, "IPaddrRCServer"  , "I"  );
      fid_status_SSID           = env->GetFieldID(cls, "SSID", "Lwdqapi/WdqApi$SSID_t;" );      
      fid_status_connStatus     = env->GetFieldID(cls, "_I_connStatus"   , "I"  );
      fid_status_channelNum     = env->GetFieldID(cls, "channelNum"      , "I"  );
      fid_status_RSSI           = env->GetFieldID(cls, "RSSI"            , "I"  );
      fid_status_SNR            = env->GetFieldID(cls, "SNR"             , "I"  );
      fid_status_bitRate        = env->GetFieldID(cls, "bitRate"         , "I"  );
      fid_status_framesTx       = env->GetFieldID(cls, "framesTx"        , "I"  );
      fid_status_framesRx       = env->GetFieldID(cls, "framesRx"        , "I"  );
      fid_status_bytesTx        = env->GetFieldID(cls, "bytesTx"         , "I"  );
      fid_status_bytesRx	     = env->GetFieldID(cls, "bytesRx"         , "I"  );
      fid_status_adapterReady   = env->GetFieldID(cls, "adapterReady"    , "Z"  );
      fid_status_probing        = env->GetFieldID(cls, "probing"         , "Z"  );
      
      ParseSSID::getFids( env );
   }
   
   env->SetIntField    (S, fid_status_verDriver     ,            _S.verDriver             );  
   env->SetIntField    (S, fid_status_verSupplicant ,            _S.verSupplicant         );      
   env->SetIntField    (S, fid_status_adapterType   ,            _S.adapterType           );      
   env->SetIntField    (S, fid_status_regDomain     ,            _S.regDomain             ); 
   env->SetIntField    (S, fid_status_IPaddr        ,            _S.IPaddr                );      
   env->SetIntField    (S, fid_status_IPaddrAP      ,            _S.IPaddrAP              );      
   env->SetIntField    (S, fid_status_IPaddrRCServer,            _S.IPaddrRCServer        );      
   env->SetIntField    (S, fid_status_connStatus    ,            _S.connStatus            );      
   env->SetIntField    (S, fid_status_channelNum    ,            _S.channelNum            );      
   env->SetIntField    (S, fid_status_RSSI          ,            _S.RSSI                  );      
   env->SetIntField    (S, fid_status_SNR           ,            _S.SNR                   );      
   env->SetIntField    (S, fid_status_bitRate       ,            _S.bitRate               );      
   env->SetIntField    (S, fid_status_framesTx      ,            _S.framesTx              );      
   env->SetIntField    (S, fid_status_framesRx      ,            _S.framesRx              );      
   env->SetIntField    (S, fid_status_bytesTx       ,            _S.bytesTx               );  
   env->SetIntField    (S, fid_status_bytesRx	    ,            _S.bytesRx               );      
   env->SetBooleanField(S, fid_status_adapterReady  , (jboolean)(_S.adapterReady != FALSE));      
   env->SetBooleanField(S, fid_status_probing       , (jboolean)(_S.probing      != FALSE));      

   // Array members require special handling
   _setObjectFromMac( _S.MAC  , env, S, fid_status_MAC   );
   _setObjectFromMac( _S.MACAP, env, S, fid_status_MACAP );   
   
   ParseSSID::setObjectFromSsid( _S.SSID, env, S, fid_status_SSID );

/*     
It would be nice to know how to set a JNI Enum object directly.)
   // enum values
   {  
      jclass       ec  =               env->FindClass( "WdqApi$radioType_e" ) ;        
      jmethodID    mid =               env->GetStaticMethodID(ec, "values", "()[Lwdqapi/WdqApi$radioType_e;");
      jobjectArray ea  = (jobjectArray)env->CallStaticObjectMethod( ec, mid ); 
      jobject      e   =               env->GetObjectArrayElement( ea, _S.adapterType );
      
      jclass   cls  = env->GetObjectClass(S);       
      jfieldID fid  = env->GetFieldID(cls, "adapterType", "Lwdqapi/WdqApi$radioType_e;" );
      jobject  jobj = env->GetObjectField( S, fid );    
      
      env->SetObjectField( jobj, fid, e ); // <-- crashes here EXCEPTION_ACCESS_VIOLATION (0xc0000005)
   }
*/
     
   return err;
}

// --------------------------------------------------------------------------------------  
/*
 * Class:     WdqApi
 * Method:    WdqSetRadioParameters
 * Signature: (Lwdqapi/WdqApi/radioStore_t;I)I
 */
 
bool bInit_radioStore_fids = true; 

jfieldID fid_radio_clientName        ;
jfieldID fid_radio_options           ;
jfieldID fid_radio_txPowerQdbm       ;
jfieldID fid_radio_powerSaveMode     ;
jfieldID fid_radio_bands             ;
jfieldID fid_radio_bgChanMask        ;
jfieldID fid_radio_bitRateMask       ;
jfieldID fid_radio_dfsChanMode       ;
jfieldID fid_radio_roamMagicFactor   ;
jfieldID fid_radio_adhocChannel      ;
jfieldID fid_radio_remoteServerIP    ;
jfieldID fid_radio_testMode          ;
jfieldID fid_radio_debugMode         ;
jfieldID fid_radio_antennaConfig     ;
jfieldID fid_radio_nullPacketTimeS   ;
jfieldID fid_radio_disconnectScanTime;
jfieldID fid_radio_fragThresh        ;
jfieldID fid_radio_rtsThresh         ;

void InitRadioStoreFids( JNIEnv *env ) {

   if(bInit_radioStore_fids) {
      bInit_radioStore_fids = false;
   
      jclass cls = env->FindClass( WDQAPICLASS(radioStore_t)); 
      fid_radio_clientName          = env->GetFieldID(cls, "clientName"         , "Ljava/lang/String;" );
      fid_radio_options             = env->GetFieldID(cls, "_I_options"         , "I"  ); // TODO
      fid_radio_txPowerQdbm         = env->GetFieldID(cls, "txPowerQdbm"        , "I"  );
      fid_radio_powerSaveMode       = env->GetFieldID(cls, "_I_powerSaveMode"   , "I"  );
      fid_radio_bands               = env->GetFieldID(cls, "_I_bands"           , "I"  ); // TODO
      fid_radio_bgChanMask          = env->GetFieldID(cls, "bgChanMask"         , "I"  );        
      fid_radio_bitRateMask         = env->GetFieldID(cls, "bitRateMask"        , "I"  );
      fid_radio_dfsChanMode         = env->GetFieldID(cls, "_I_dfsChanMode"     , "I"  );
      fid_radio_roamMagicFactor     = env->GetFieldID(cls, "roamMagicFactor"    , "I"  );
      fid_radio_adhocChannel        = env->GetFieldID(cls, "adhocChannel"       , "I"  );      
      fid_radio_remoteServerIP      = env->GetFieldID(cls, "remoteServerIP"     , "I"  );
      fid_radio_testMode            = env->GetFieldID(cls, "_I_testMode"        , "I"  );
      fid_radio_debugMode           = env->GetFieldID(cls, "_I_debugMode"       , "I"  );
      fid_radio_antennaConfig       = env->GetFieldID(cls, "_I_antennaConfig"   , "I"  );
      fid_radio_nullPacketTimeS     = env->GetFieldID(cls, "nullPacketTimeS"    , "I"  );
      fid_radio_disconnectScanTime  = env->GetFieldID(cls, "disconnectScanTime" , "I"  );
      fid_radio_fragThresh          = env->GetFieldID(cls, "fragThresh"         , "I"  );
      fid_radio_rtsThresh           = env->GetFieldID(cls, "rtsThresh"          , "I"  );
   }
}

 
JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqSetRadioParameters( JNIEnv *env, jobject, jobject S, jint flags ) {
   
   InitRadioStoreFids( env );
   
   wdq_radioStore_t _S;
   _S.options            = (wdq_radioOption_e)  env->GetIntField(S, fid_radio_options            );      
   _S.txPowerQdbm        =                      env->GetIntField(S, fid_radio_txPowerQdbm        );      
   _S.powerSaveMode      = (wdq_powerSaveMode_e)env->GetIntField(S, fid_radio_powerSaveMode      ); 
   _S.bands              = (wdq_bandSelect_e)   env->GetIntField(S, fid_radio_bands              );      
   _S.bgChanMask         =                      env->GetIntField(S, fid_radio_bgChanMask         );      
   _S.bitRateMask        =                      env->GetIntField(S, fid_radio_bitRateMask        );      
   _S.dfsChanMode        = (wdq_dfsChanMode_e)  env->GetIntField(S, fid_radio_dfsChanMode        );      
   _S.roamMagicFactor    =                      env->GetIntField(S, fid_radio_roamMagicFactor    );      
   _S.adhocChannel       =                      env->GetIntField(S, fid_radio_adhocChannel       );      
   _S.remoteServerIP     =                      env->GetIntField(S, fid_radio_remoteServerIP     );      
   _S.testMode           = (wdq_testModes_e)    env->GetIntField(S, fid_radio_testMode           );      
   _S.debugMode          = (wdq_debugType_e)    env->GetIntField(S, fid_radio_debugMode          );      
   _S.antennaConfig      = (wdq_antennaSelect_e)env->GetIntField(S, fid_radio_antennaConfig      );      
   _S.nullPacketTimeS    =                      env->GetIntField(S, fid_radio_nullPacketTimeS    );      
   _S.disconnectScanTime =                      env->GetIntField(S, fid_radio_disconnectScanTime );      
   _S.fragThresh         =                      env->GetIntField(S, fid_radio_fragThresh         );      
   _S.rtsThresh          =                      env->GetIntField(S, fid_radio_rtsThresh          );      

   // String members
   _setWdqFromString( _S.clientName, NELEM(_S.clientName), env, S, fid_radio_clientName );   
   
   return WdqSetRadioParameters( &_S, (wdq_apiFlags_e)flags );
}

// --------------------------------------------------------------------------------------  

/*
 * Class:     WdqApi
 * Method:    WdqGetScanList
 * Signature: (Lwdqapi/WdqApi/scanList_t;I)I
 */
 
bool bInit_scanList_fids = true;
jfieldID fid_scanList_elemCount;
jfieldID fid_scanList_elemList ;

bool bInit_scanElem_fids = true;
jfieldID fid_scanElement_SSID       ;
jfieldID fid_scanElement_MAC        ;
jfieldID fid_scanElement_netType    ;
jfieldID fid_scanElement_options    ;
jfieldID fid_scanElement_band       ;
jfieldID fid_scanElement_channel    ;
jfieldID fid_scanElement_RSSI       ;
jfieldID fid_scanElement_bitRateKHz ;

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqGetScanList(JNIEnv *env, jobject, jobject S, jint reserved) {


   if(bInit_scanList_fids) {
      bInit_scanList_fids = false;
      
      jclass cls = env->GetObjectClass(S);    
      fid_scanList_elemCount     = env->GetFieldID(cls, "elemCount"    , "I"                       );
      fid_scanList_elemList      = env->GetFieldID(cls, "elemList"     , "[Lwdqapi/WdqApi$scanElement_t;" );
   }
   
   wdq_scanList_t _S;
   int err = WdqGetScanList( &_S, reserved );
   env->SetIntField(S, fid_scanList_elemCount, _S.elemCount  );         
  
   jobjectArray a = (jobjectArray)env->GetObjectField( S, fid_scanList_elemList ); 
   for (int i=0; i<_S.elemCount; i++) {
      jobject obj = env->GetObjectArrayElement(a, i);
      
      if(bInit_scanElem_fids) {
         bInit_scanElem_fids = false;
         jclass  cls = env->GetObjectClass(obj);          
         fid_scanElement_SSID       = env->GetFieldID(cls, "SSID"         , "Lwdqapi/WdqApi$SSID_t;"         );
         fid_scanElement_MAC        = env->GetFieldID(cls, "MAC"          , "[B"                      );
         fid_scanElement_netType    = env->GetFieldID(cls, "_I_netType"   , "I"                       );
         fid_scanElement_options    = env->GetFieldID(cls, "_I_options"   , "I"                       );
         fid_scanElement_band       = env->GetFieldID(cls, "_I_band"      , "I"                       );
         fid_scanElement_channel    = env->GetFieldID(cls, "channel"      , "I"                       );
         fid_scanElement_RSSI       = env->GetFieldID(cls, "RSSI"         , "I"                       );
         fid_scanElement_bitRateKHz = env->GetFieldID(cls, "bitRateKHz"   , "I"                       );
         ParseSSID::getFids( env ); 
      }
      
      wdq_scanElement_t &elem = _S.elemList[i];      
      ParseSSID::setObjectFromSsid( elem.SSID, env, obj, fid_scanElement_SSID     );   
                _setObjectFromMac ( elem.MAC , env, obj, fid_scanElement_MAC      );
      env->SetIntField(obj, fid_scanElement_netType   , elem.netType    );   
      env->SetIntField(obj, fid_scanElement_options   , elem.options    );   
      env->SetIntField(obj, fid_scanElement_band      , elem.band       );   
      env->SetIntField(obj, fid_scanElement_channel   , elem.channel    );   
      env->SetIntField(obj, fid_scanElement_RSSI      , elem.RSSI       );   
      env->SetIntField(obj, fid_scanElement_bitRateKHz, elem.bitRateKHz );                     
   }
   
   return err;
}

// --------------------------------------------------------------------------------------  

/*
 * Class:     WdqApi
 * Method:    WdqGetNetwork
 * Signature: (Ljava/lang/String;Lwdqapi/WdqApi/netStoreElem_t;I)I
 */

bool bInit_netStoreElem_fids = true;

jfieldID fid_netStoreElem_options      ;
jfieldID fid_netStoreElem_order        ;
jfieldID fid_netStoreElem_pmkCacheType ;
jfieldID fid_netStoreElem_SSID         ;
jfieldID fid_netStoreElem_name         ;
jfieldID fid_netStoreElem_authType80211;
jfieldID fid_netStoreElem_passwordType ;
jfieldID fid_netStoreElem_wpaVersion   ;
jfieldID fid_netStoreElem_eapMethods   ;
jfieldID fid_netStoreElem_eapOptions   ;
jfieldID fid_netStoreElem_authHints    ;
jfieldID fid_netStoreElem_userIdentity ;
jfieldID fid_netStoreElem_keyFile      ;
jfieldID fid_netStoreElem_password     ;
jfieldID fid_netStoreElem_keyFilePwd   ;
jfieldID fid_netStoreElem_wepTxKey     ;
jfieldID fid_netStoreElem_wepKeyLen    ;
jfieldID fid_netStoreElem_wepKey       ;

void InitNetStoreElemFids( JNIEnv *env ) {

   if(bInit_netStoreElem_fids) {
      bInit_netStoreElem_fids = false;
      
      jclass cls = env->FindClass( WDQAPICLASS(netStoreElem_t));       
      
      fid_netStoreElem_options       = env->GetFieldID(cls, "_I_options"      , "I"                  );
      fid_netStoreElem_order         = env->GetFieldID(cls, "order"           , "I"                  );
      fid_netStoreElem_pmkCacheType  = env->GetFieldID(cls, "_I_pmkCacheType" , "I"                  );
      fid_netStoreElem_SSID          = env->GetFieldID(cls, "SSID"            , "Lwdqapi/WdqApi$SSID_t;"    );
      fid_netStoreElem_name          = env->GetFieldID(cls, "name"            , "Ljava/lang/String;" );
      fid_netStoreElem_authType80211 = env->GetFieldID(cls, "_I_authType80211", "I"                  );
      fid_netStoreElem_passwordType  = env->GetFieldID(cls, "_I_passwordType" , "I"                  );
      fid_netStoreElem_wpaVersion    = env->GetFieldID(cls, "_I_wpaVersion"   , "I"                  );
      fid_netStoreElem_eapMethods    = env->GetFieldID(cls, "_I_eapMethods"   , "I"                  );
      fid_netStoreElem_eapOptions    = env->GetFieldID(cls, "_I_eapOptions"   , "I"                  );
      fid_netStoreElem_authHints     = env->GetFieldID(cls, "_I_authHints"    , "I"                  );
      fid_netStoreElem_userIdentity  = env->GetFieldID(cls, "userIdentity"    , "Ljava/lang/String;" );
      fid_netStoreElem_password      = env->GetFieldID(cls, "password"        , "Ljava/lang/String;" );
      fid_netStoreElem_keyFilePwd    = env->GetFieldID(cls, "keyFilePwd"      , "Ljava/lang/String;" );
      fid_netStoreElem_keyFile       = env->GetFieldID(cls, "keyFile"         , "Ljava/lang/String;" );
      fid_netStoreElem_wepTxKey      = env->GetFieldID(cls, "wepTxKey"        , "I"                  );
      fid_netStoreElem_wepKeyLen     = env->GetFieldID(cls, "wepKeyLen"       , "[I"                 );
      fid_netStoreElem_wepKey        = env->GetFieldID(cls, "wepKey"          , "[[B"                );
      
      ParseSSID::getFids( env );
   }
}

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqGetNetwork(JNIEnv *env, jobject, jstring name, jobject S, jint flags) {

   InitNetStoreElemFids( env );
   wdq_netStoreElem_t _S;
   
   const char *szName = env->GetStringUTFChars( name, 0);    
   int err = WdqGetNetwork( CvtStrWdq( szName ), &_S, (wdq_apiFlags_e)flags );
   env->ReleaseStringUTFChars( name, szName );          
   
   env->SetIntField(S, fid_netStoreElem_options      , _S.options      );         
   env->SetIntField(S, fid_netStoreElem_order        , _S.order        ); 
   env->SetIntField(S, fid_netStoreElem_pmkCacheType , _S.pmkCacheType );
   env->SetIntField(S, fid_netStoreElem_authType80211, _S.pwdInfo.authType80211 );   
   env->SetIntField(S, fid_netStoreElem_passwordType , _S.pwdInfo.passwordType  );   
   env->SetIntField(S, fid_netStoreElem_wpaVersion   , _S.pwdInfo.wpaVersion    );   
   env->SetIntField(S, fid_netStoreElem_eapMethods   , _S.pwdInfo.eapMethods    );   
   env->SetIntField(S, fid_netStoreElem_eapOptions   , _S.pwdInfo.eapOptions    );   
   env->SetIntField(S, fid_netStoreElem_authHints    , _S.pwdInfo.authHints     );   
   env->SetIntField(S, fid_netStoreElem_wepTxKey     , _S.pwdInfo.priv.wepKeys.txKey );         
   
   ParseSSID::setObjectFromSsid( _S.SSID, env, S, fid_netStoreElem_SSID );   
   
   _setStringFromWdq( env, S, fid_netStoreElem_name        , _S.name                    );
   _setStringFromWdq( env, S, fid_netStoreElem_keyFile     , _S.pwdInfo.keyFile         );   
   _setStringFromAsc( env, S, fid_netStoreElem_userIdentity, _S.pwdInfo.userIdentity    );         
   _setStringFromAsc( env, S, fid_netStoreElem_password    , _S.pwdInfo.priv.password   );         
   _setStringFromAsc( env, S, fid_netStoreElem_keyFilePwd  , _S.pwdInfo.priv.keyFilePwd );      
      
   // int[]
   {  int n = NELEM(_S.pwdInfo.priv.wepKeys.keylen);
      jintArray a = (jintArray) env->GetObjectField( S, fid_netStoreElem_wepKeyLen );   
      env->SetIntArrayRegion( a, 0, n, (jint*)&_S.pwdInfo.priv.wepKeys.keylen[0] );      
   }   
   
   // byte[][]  
   {  int nKeys = NELEM( _S.pwdInfo.priv.wepKeys.key );   
      jobjectArray aa = (jobjectArray)env->GetObjectField( S, fid_netStoreElem_wepKey );   
      for (int i=0; i<nKeys; i++) {
         int nLen = min( _S.pwdInfo.priv.wepKeys.keylen[i], NELEM(_S.pwdInfo.priv.wepKeys.key[i]));
         jbyteArray a = (jbyteArray)env->GetObjectArrayElement( aa, i );               
         env->SetByteArrayRegion( a, 0, nLen, (jbyte*)&_S.pwdInfo.priv.wepKeys.key[i][0] );            
      }
   } 

   return err;
}
 
void _setNetworkElement( wdq_netStoreElem_t &_S, JNIEnv *env, jobject S ) {
 
   InitNetStoreElemFids( env );
   
   _S.options                    = (wdq_netElemOpts_e  )env->GetIntField(S, fid_netStoreElem_options       );         
   _S.order                      =                      env->GetIntField(S, fid_netStoreElem_order         ); 
   _S.pmkCacheType               = (wdq_pmkCacheType_e )env->GetIntField(S, fid_netStoreElem_pmkCacheType  );
   _S.pwdInfo.authType80211      = (wdq_authType80211_e)env->GetIntField(S, fid_netStoreElem_authType80211 );   
   _S.pwdInfo.passwordType       = (wdq_passwordType_e )env->GetIntField(S, fid_netStoreElem_passwordType  );   
   _S.pwdInfo.wpaVersion         = (wdq_wpaType_e      )env->GetIntField(S, fid_netStoreElem_wpaVersion    );   
   _S.pwdInfo.eapMethods         = (wdq_eapType_e      )env->GetIntField(S, fid_netStoreElem_eapMethods    );   
   _S.pwdInfo.eapOptions         = (wdq_eapOpts_e      )env->GetIntField(S, fid_netStoreElem_eapOptions    );   
   _S.pwdInfo.authHints          = (wdq_authHint_e     )env->GetIntField(S, fid_netStoreElem_authHints     );   
   _S.pwdInfo.priv.wepKeys.txKey =                      env->GetIntField(S, fid_netStoreElem_wepTxKey      );         
   
   ParseSSID::setSsidFromObject( _S.SSID, env, S, fid_netStoreElem_SSID );   
   _ppJNI.printf( "SSID = %s(%d)\n", _S.SSID.name, _S.SSID.len ); 
   
   _setWdqFromString( _S.name                   , NELEM(_S.name                   ), env, S, fid_netStoreElem_name         );
   _setWdqFromString( _S.pwdInfo.keyFile        , NELEM(_S.pwdInfo.keyFile        ), env, S, fid_netStoreElem_keyFile      );   
   _setAscFromString( _S.pwdInfo.userIdentity   , NELEM(_S.pwdInfo.userIdentity   ), env, S, fid_netStoreElem_userIdentity );         
   _setAscFromString( _S.pwdInfo.priv.password  , NELEM(_S.pwdInfo.priv.password  ), env, S, fid_netStoreElem_password     );         
   _setAscFromString( _S.pwdInfo.priv.keyFilePwd, NELEM(_S.pwdInfo.priv.keyFilePwd), env, S, fid_netStoreElem_keyFilePwd   );    
      
   //_getWepKeyLens( S, fid_netStoreElem_wepKeyLen, _S.pwdInfo.priv.wepKeys.keylen );   
   {  jintArray a = (jintArray)env->GetObjectField( S, fid_netStoreElem_wepKeyLen );
      memset( _S.pwdInfo.priv.wepKeys.keylen, 0, sizeof _S.pwdInfo.priv.wepKeys.keylen );
      int n = min( env->GetArrayLength( a ), NELEM(_S.pwdInfo.priv.wepKeys.keylen));
      env->GetIntArrayRegion( a, 0, n, (jint*)_S.pwdInfo.priv.wepKeys.keylen );      
   }   
   
   
   //_getWepKeys( S, fid_netStoreElem_wepKey, _S.pwdInfo.priv.wepKeys.key );      
   {  jobjectArray aa = (jobjectArray)env->GetObjectField( S, fid_netStoreElem_wepKey );   
      memset( _S.pwdInfo.priv.wepKeys.key, 0, sizeof _S.pwdInfo.priv.wepKeys.key );
      int n = min( env->GetArrayLength( aa ), NELEM( _S.pwdInfo.priv.wepKeys.key ));   
      int maxKeyLen = NELEM(_S.pwdInfo.priv.wepKeys.key[0]);
      for (int i=0; i<n; i++) {
         jbyteArray a = (jbyteArray)env->GetObjectArrayElement( aa, i );         
         int nLen = min( _S.pwdInfo.priv.wepKeys.keylen[i], maxKeyLen );
         env->GetByteArrayRegion( a, 0, nLen, (jbyte*)_S.pwdInfo.priv.wepKeys.key[i] );                     
      }
   } 
}   

// --------------------------------------------------------------------------------------
   
/*
 * Class:     WdqApi
 * Method:    WdqAddNetwork
 * Signature: (Lwdqapi/WdqApi/netStoreElem_t;I)I
 */
JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqAddNetwork(JNIEnv *env, jobject, jobject S, jint flags) {
   wdq_netStoreElem_t _S;
   _setNetworkElement( _S, env, S );
   return WdqAddNetwork( &_S, (wdq_apiFlags_e)flags );
}

// --------------------------------------------------------------------------------------

/*
 * Class:     WdqApi
 * Method:    WdqChangeNetwork
 * Signature: (Ljava/lang/String;Lwdqapi/WdqApi/netStoreElem_t;I)I
 */
 
JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqChangeNetwork(JNIEnv *env, jobject, jstring name, jobject S, jint flags) {
   wdq_netStoreElem_t _S;
   _setNetworkElement( _S, env, S );
   
   const char *szName = env->GetStringUTFChars( name, 0 );    
   int err = WdqChangeNetwork( CvtStrWdq( szName ), &_S, (wdq_apiFlags_e)flags );
   env->ReleaseStringUTFChars( name, szName );
   
   return err;
}

// --------------------------------------------------------------------------------------

/*
 * Class:     WdqApi
 * Method:    WdqDelNetwork
 * Signature: (Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqDelNetwork(JNIEnv *env, jobject, jstring name, jint flags) {

   const char *szName = env->GetStringUTFChars( name, 0 );    
   int err = WdqDeleteNetwork( CvtStrWdq( szName ), (wdq_apiFlags_e)flags );
   env->ReleaseStringUTFChars( name, szName );             
   
   return err;
}

// --------------------------------------------------------------------------------------

/*
 * Class:     WdqApi
 * Method:    WdqGetConfiguration
 * Signature: (Lwdqapi/WdqApi/radioData_t;I)I
 */
 
bool bInit_radioData_fids = true;

jfieldID fid_radioData_networkStore ;    
jfieldID fid_radioData_radioStore   ;    
//jfieldID fid_radioDataSpecStore ;    

jfieldID fid_netStore_autoSwitch     ;
jfieldID fid_netStore_currentNetwork ;
jfieldID fid_netStore_networkCount   ;
jfieldID fid_netStore_networkList    ;
jfieldID fid_netStore_certStorePath  ;

JNIEXPORT jint JNICALL Java_wdqapi_WdqApi_WdqGetConfiguration(JNIEnv *env, jobject, jobject S, jint flags) {

   if(bInit_radioData_fids) {
      bInit_radioData_fids = false;
      
      jclass cls = env->FindClass(WDQAPICLASS(radioData_t));          
      fid_radioData_networkStore = env->GetFieldID(cls, "networks"  , "Lwdqapi/WdqApi$networkStore_t;" );
      fid_radioData_radioStore   = env->GetFieldID(cls, "radioStore", "Lwdqapi/WdqApi$radioStore_t;"   );
      
      cls = env->FindClass( WDQAPICLASS(networkStore_t));
      fid_netStore_autoSwitch     = env->GetFieldID(cls, "autoSwitch"    , "Z"                   );
      fid_netStore_currentNetwork = env->GetFieldID(cls, "currentNetwork", "Ljava/lang/String;"  );
      fid_netStore_networkCount   = env->GetFieldID(cls, "networkCount"  , "I"                   );
      fid_netStore_networkList    = env->GetFieldID(cls, "networkList"   , "[Ljava/lang/String;" );
      fid_netStore_certStorePath  = env->GetFieldID(cls, "certStorePath" , "Ljava/lang/String;"  );
      
      InitNetStoreElemFids( env );
      InitRadioStoreFids  ( env );
   }   

   wdq_radioData_t _S;
   int err = WdqGetConfiguration( &_S, (wdq_apiFlags_e)flags );
   if (WDQ_APIERR_SUCCESS != err) {
      return err;
   }
   
   jobject NS = env->GetObjectField( S, fid_radioData_networkStore );
   wdq_networkStore_t &_NS = _S.networks;
   env->SetBooleanField(   NS, fid_netStore_autoSwitch    , !!_NS.autoSwitch   );      
   env->SetIntField    (   NS, fid_netStore_networkCount  ,   _NS.networkCount   );   
   _setStringFromWdq( env, NS, fid_netStore_currentNetwork,   _NS.currentNetwork );
   _setStringFromWdq( env, NS, fid_netStore_certStorePath ,   _NS.certStorePath  );
   
   jobjectArray a = (jobjectArray)env->GetObjectField( NS, fid_netStore_networkList );    
   for (int i=0; i<min( _NS.networkCount, NELEM( _NS.networkList )); i++) {
      jstring s = env->NewStringUTF( CvtStrA( _NS.networkList[i] )); 
      env->SetObjectArrayElement( a, i, s );
   }
   
   jobject RS = env->GetObjectField( S, fid_radioData_radioStore );   
   wdq_radioStore_t &_RS = _S.radioStore;   
   env->SetIntField (      RS, fid_radio_options           , _RS.options            );
   env->SetIntField (      RS, fid_radio_txPowerQdbm       , _RS.txPowerQdbm        );
   env->SetIntField (      RS, fid_radio_powerSaveMode     , _RS.powerSaveMode      );
   env->SetIntField (      RS, fid_radio_bands             , _RS.bands              );
   env->SetIntField (      RS, fid_radio_bgChanMask        , _RS.bgChanMask         );
   env->SetIntField (      RS, fid_radio_bitRateMask       , _RS.bitRateMask        );
   env->SetIntField (      RS, fid_radio_dfsChanMode       , _RS.dfsChanMode        );
   env->SetIntField (      RS, fid_radio_roamMagicFactor   , _RS.roamMagicFactor    );
   env->SetIntField (      RS, fid_radio_adhocChannel      , _RS.adhocChannel       );
   env->SetIntField (      RS, fid_radio_remoteServerIP    , _RS.remoteServerIP     );
   env->SetIntField (      RS, fid_radio_testMode          , _RS.testMode           );
   env->SetIntField (      RS, fid_radio_debugMode         , _RS.debugMode          );
   env->SetIntField (      RS, fid_radio_antennaConfig     , _RS.antennaConfig      );
   env->SetIntField (      RS, fid_radio_nullPacketTimeS   , _RS.nullPacketTimeS    );
   env->SetIntField (      RS, fid_radio_disconnectScanTime, _RS.disconnectScanTime );
   env->SetIntField (      RS, fid_radio_fragThresh        , _RS.fragThresh         );
   env->SetIntField (      RS, fid_radio_rtsThresh         , _RS.rtsThresh          );
   _setStringFromWdq( env, RS, fid_radio_clientName        , _RS.clientName         );   
   
   return WDQ_APIERR_SUCCESS;
}
