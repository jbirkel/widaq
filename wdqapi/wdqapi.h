// ============================================================================
//
// wdqapi.h 
//
//    "C" header file that declares the public members of the WDQAPI DLL
//
// NOTE: This file is intended to be cross-platform compatible.
//
// ============================================================================


#ifndef WDQAPI_H
#define WDQAPI_H 1

#ifdef __cplusplus
extern "C" {
#endif

// Ensure that _WIN32 is defined in your project for Windows CE support 
#ifdef _WIN32
  #include <windows.h> 
  #if defined(WDQAPI_STATIC_LIB)
    #define DllExport WINAPI
  #elif defined(WDQAPIDLL_EXPORTS)
    #define DllExport __declspec(dllexport) WINAPI
  #else
    #define DllExport __declspec(dllimport) WINAPI
  #endif
#else
  #define DllExport 
#endif


#define WDQ_NUM_WEP_KEYS_MAX       4
#define WDQ_WEP_KEY_SIZE          13 
#define WDQ_MAC_ADDR_SIZE          6
#define WDQ_MAX_NUM_NETWORKS      16
#define WDQ_MAX_SSID_SIZE         32
#define WDQ_MAX_NAME_SIZE         20 // used for userIdentity, network name and client name
#define WDQ_MAX_PASSWORD_SIZE     64
#define WDQ_MAX_PROBE_AP          64
#define WDQ_MAX_PATH             260  


// -----------------------------------------------------------------------------------------
// API Errors
// -----------------------------------------------------------------------------------------
#define WDQAPIERR int

#define WDQ_APIERR_SUCCESS      0   
#define WDQ_APIERR_NULLPTR     -1   // A non-optional pointer argument was NULL
#define WDQ_APIERR_BADARG      -2   // An argument value was invalid.
#define WDQ_APIERR_BADSIZE     -3   // The size field of a passed-in structure was invalid.
#define WDQ_APIERR_GENERAL     -4   // General operation-failed message. 
#define WDQ_APIERR_MEMORY      -5   // Memory allocation failed.
#define WDQ_APIERR_BADWRITE    -6   // A write operation failed.
#define WDQ_APIERR_BADREAD     -7   // A read operation failed.
#define WDQ_APIERR_NOTFOUND    -8   // An item was not found. 
#define WDQ_APIERR_BADOPEN     -9   // A resource open operation error.
#define WDQ_APIERR_COLLISION  -10   // An item was found with the same name.
#define WDQ_APIERR_BADCREATE  -11   // A resource could not be created.
#define WDQ_APIERR_CFGLOCKED  -12   // Operation did not complete because config is locked. 
#define WDQ_APIERR_MOREDATA   -13   // Passed-in buffer used to capacity, but too small for all data.
#define WDQ_APIERR_NOTSUPP    -14   // API not supported on this platform.
           

// bitRateMask is a 32 bit unsigned integer bit mask used to enable/disable various rates:
// - the upper 16 bits are for mcs indexed N rates and the lower are for cck/ofdm rates
// -----------------------------------------------------------------------------------------
//                | CCK:             | OFDM:                               |       |  MCS  |
// Bit Number     | 0 | 1 |  2  |  3 | 4 | 5 |  6 |  7 |  8 |  9 | 10 | 11 | 12-15 | 16-31 |
// Bit Rate, Mbps | 1 | 2 | 5.5 | 11 | 6 | 9 | 12 | 18 | 24 | 36 | 48 | 54 |   X   |       |
// -----------------------------------------------------------------------------------------
#define WDQ_BITRATE_CCK_ONLY  0x0000000f
#define WDQ_BITRATE_OFDM_ONLY 0x00000ff0
#define WDQ_BITRATE_MCS_ONLY  0xffff0000

#define WDQ_BITRATE_CCK_OFDM  (WDQ_BITRATE_CCK_ONLY | WDQ_BITRATE_OFDM_ONLY)
#define WDQ_BITRATE_ALL       (WDQ_BITRATE_MCS_ONLY | WDQ_BITRATE_CCK_OFDM)


// bgChanMask is a 32 bit unsigned integer bit mask used to to enable/disable which channels 
// to scan and connect on.
// -----------------------------------------------------------------------------------------
// Bit Number     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9  | 10 | 11 | 12 | 13 | 14-31 |
// B/G Channel    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 |   X   |
// -----------------------------------------------------------------------------------------
#define WDQ_BGCHAN_ALL   0x3fff


// Some common transmit power levels.
// NOTE: TxPower field is numeric in quarter-dBm units with -1=Max
#define WDQ_TXPWR_1       4   //  1 dBM
#define WDQ_TXPWR_5      20   //  5 dBM
#define WDQ_TXPWR_10     40	// 10 dBM
#define WDQ_TXPWR_MAX    -1   // Maximum power

// Min/max range of roamMagicFactor field.
#define WDQ_ROAMMAGIC_MIN  1
#define WDQ_ROAMMAGIC_MAX 10

// Ad Hoc Channel range
#define WDQ_ADHOCCHAN_MIN  1
#define WDQ_ADHOCCHAN_MAX 14


//
// API Character type (for strings)
//    

#if   defined(WDQAPI_WIDE_CHAR_IFC)
   #define WDQ_WIDE_CHAR
#elif defined(WDQAPI_UTF8_CHAR_IFC)
   #define WDQ_UTF8_CHAR
#else      // DEFAULT
   #define WDQ_WIDE_CHAR
#endif

#ifdef WDQ_WIDE_CHAR
typedef wchar_t wdq_Char_t  ;  // Unicode
#define         wdq_Char_size 2
//#define xWDQ_CH(s) L ## s
//#define  WDQ_CH(s) xWDQ_CH(s)
#define  WDQ_CH(s) L ## s

#define wdqStrNCpy wcsncpy
#define wdqStrCpy   wcscpy
#else 
typedef char    wdq_Char_t  ;  // UTF-8 
#define         wdq_Char_size 1
#define  WDQ_CH(s) s

#define wdqStrNCpy strncpy
#define wdqStrCpy   strcpy
#endif

#define WDQ_CHBUF_SIZE(n) ((n)*wdq_Char_size)

//
// Special API integral types.
//
typedef char          wdq_Ascii_t ;  // 7-bit ASCII character data
typedef unsigned char wdq_Byte_t  ;  // unformatted, or raw, 8-bit values
typedef unsigned int  wdq_Bool_t  ;  // 0 = false, non-zero = true
typedef unsigned int  wdq_IPaddr_t;  // high byte is leftmost digit (E.g. 0x01020304 is 1.2.3.4)

typedef struct { 
  int      	 len;   
  wdq_Byte_t name[WDQ_MAX_SSID_SIZE];
} wdq_SSID_t;  
  

// For wdq_Bool_t:
#ifndef FALSE
   #define FALSE 0
   #define TRUE  1
#endif    

// Fixed-length string types
typedef wdq_Char_t wdq_networkName_t[WDQ_CHBUF_SIZE(WDQ_MAX_NAME_SIZE+1)];

//
// -----------------------------------------------------------------------------------------
// Enumerated Types
// -----------------------------------------------------------------------------------------
//
// Notes:
// -- "NOVALUE" types can be used to indicate not-initialized or invalid value
// -- "BOUNDARY" types mark the end of the list
//

// Power-save modes.
typedef enum  {
	WDQ_PSAVE_NOVALUE = 0,
	WDQ_PSAVE_OFF,
	WDQ_PSAVE_FAST,
	WDQ_PSAVE_MAX,
	WDQ_PSAVE_BOUNDARY 
}  wdq_powerSaveMode_e;

// Frequency bands
typedef enum  {
	WDQ_BAND_NOVALUE = 0,
	WDQ_BAND_B = 1<<0,	
	WDQ_BAND_G = 1<<1,
	WDQ_BAND_A = 1<<2,	
	WDQ_BAND_N = 1<<3,
	WDQ_BAND_BOUNDARY = 1<<4,
	WDQ_BAND_BG  = WDQ_BAND_B | WDQ_BAND_G,		
	WDQ_BAND_ABG = WDQ_BAND_A | WDQ_BAND_BG,		
} wdq_bandSelect_e ; 

// Misc. boolean radio config parameters
typedef enum {
   WDQ_RADOPT_NOVALUE = 0,
   WDQ_RADOPT_DISABLEAUTOCONNECT = 1<<0,   
   WDQ_RADOPT_REMOTECFGENABLE    = 1<<1,
   WDQ_RADOPT_REMOTESRVAUTO      = 1<<2,
} wdq_radioOption_e ;

// Determines how to use the DFS channels
typedef enum {
   WDQ_DFSMOD_NOVALUE = 0,
   WDQ_DFSMODE_OFF,   // don't scan or use DFS channels
   WDQ_DFSMODE_ON ,
} wdq_dfsChanMode_e ;

// Regulatory Domains
typedef enum  {
	WDQ_REG_NOVALUE = 0,
	WDQ_REG_AUTO,
	WDQ_REG_TELEC,
	WDQ_REG_ETSI,
	WDQ_REG_FCC,
	WDQ_REG_KOREA,
	WDQ_REG_BOUNDARY 
} wdq_regDomain_e;

// Antenna configuration
typedef enum {
	WDQ_ANTENNA_NOVALUE = 0,
	WDQ_ANTENNA_0_ONLY,
	WDQ_ANTENNA_1_ONLY,
	WDQ_ANTENNA_BOTH
} wdq_antennaSelect_e;

// Network element option bits
typedef enum  {
	WDQ_NEOPTS_NOVALUE      = 0,
	WDQ_NEOPTS_ADHOC        = 1<<0,	 // 0-Infrastructure, 1-AdHoc 
	WDQ_NEOPTS_NOAUTOSWITCH = 1<<1,	 // 0-Allow auto-switch to this network, 1-Don't
	WDQ_NEOPTS_BOUNDARY,		
} wdq_netElemOpts_e ; 

// Pairwise Master Key Caching methods
typedef enum {
   WDQ_PMKTYPE_NOVALUE = 0,
   WDQ_PMKTYPE_PMK ,     // PMK Caching
   WDQ_PMKTYPE_OKC ,     // Opportunistic PMK Caching
   WDQ_PMKTYPE_CCKM,     // Cisco Centralized Key Management
   WDQ_PMKTYPE_BOUNDARY,
} wdq_pmkCacheType_e;

// Security types
typedef enum {
	WDQ_PWDTYPE_NOVALUE = 0,
	WDQ_PWDTYPE_NONE ,   // open
	WDQ_PWDTYPE_WEP  ,   // static WEP
	WDQ_PWDTYPE_PSK  ,   // Personal WPA
	WDQ_PWDTYPE_WPA  ,   // Enterprise WPA
	WDQ_PWDTYPE_EAP  ,   // 802.1x
	WDQ_PWDTYPE_BOUNDARY
} wdq_passwordType_e;

// 802.11 Authentication methods
typedef enum {
	WDQ_AUTHTYPE_NOVALUE = 0,
	WDQ_AUTHTYPE_OPEN,
	WDQ_AUTHTYPE_SHARED,
	WDQ_AUTHTYPE_NETWORKEAP,
	WDQ_AUTHTYPE_BOUNDARY
} wdq_authType80211_e;

// WPA version configuration (1, 2 or both)
typedef enum {
	WDQ_WPATYPE_NOVALUE = 0,
	WDQ_WPATYPE_V1_ONLY,  // Only WPA1
	WDQ_WPATYPE_V2_ONLY,  // Only WPA2
	WDQ_WPATYPE_VER_ANY,  // WPA1 or WPA2
	WDQ_WPATYPE_BOUNDARY
} wdq_wpaType_e;

// EAP Methods.  (There can be more than one.)
typedef enum {
	WDQ_EAP_NOVALUE     = 0   ,
	WDQ_EAP_NONE        = 1<<0,
	WDQ_EAP_LEAP        = 1<<1,
	WDQ_EAP_EAPFAST     = 1<<2,
	WDQ_EAP_PEAPGTC     = 1<<3,
	WDQ_EAP_PEAPMSCHAP  = 1<<4,
	WDQ_EAP_PEAPTLS     = 1<<5,
	WDQ_EAP_EAPTLS      = 1<<6,
	WDQ_EAP_BOUNDARY    = 1<<7,
} wdq_eapType_e;

// EAP Method options.  (There can be more than one.)
typedef enum {
   WDQ_EAPOPT_NOVALUE               = 0,
   WDQ_EAPOPT_FORCE_SERVER_VALIDATE = 1<<0,
   WDQ_EAPOPT_ALLOW_ANON_PROVISION  = 1<<1,
   WDQ_EAPOPT_ALLOW_PAC_PROVISION   = 1<<2,
} wdq_eapOpts_e;   

// Authentication Hints  (There can be more than one.)
typedef enum {
   WDQ_AUTHHINT_NOVALUE          = 0,
   WDQ_AUTHHINT_USE_SAVED_CREDS  = 1<<0,   // config contains saved login name and password
   WDQ_AUTHHINT_USE_KEY_FILE     = 1<<1,   // config contains saved key file name and (maybe) a password   
   //WDQ_AUTHHINT_NEW_LOGIN_PWD    = 1<<2,   // login password has been changed
   //WDQ_AUTHHINT_NEW_KEYFILE_PWD  = 1<<3,   // key file password has been changed
   //WDQ_AUTHHINT_NEW_WEP_KEYS     = 1<<4,   // wep keys have been changed
} wdq_authHint_e; 

// Identifiers for specific radios or radio types
typedef enum {
	WDQ_RADIO_NOVALUE = 0,
	WDQ_RADIO_UNKNOWN,
	WDQ_RADIO_ABGN_BT,
	WDQ_RADIO_BOUNDARY	
} wdq_radioType_e;

// Connection Status
typedef enum {
   WDQ_COSTAT_NOVALUE = 0,
   WDQ_COSTAT_NOT_CONNECTED,   
   WDQ_COSTAT_ASSOCIATED,    // link-up with an AP
   WDQ_COSTAT_CONNECTED,     // ready to go (authenticated, if applicable)
   
   WDQ_COSTAT_BOUNDARY,
} wdq_connStatus_e;

// Debug logging output options
typedef enum {
	WDQ_DEBUG_NOVALUE = 0,
	WDQ_DEBUG_OFF,
	WDQ_DEBUG_FILE,
	WDQ_DEBUG_RETAIL,
	WDQ_DEBUG_UDP,	
	WDQ_DEBUG_BOUNDARY
} wdq_debugType_e;

// Special test modes
typedef enum {
	WDQ_TESTING_NOVALUE = 0,
	WDQ_TESTING_OFF,
	WDQ_TESTING_RECEIVE,
	WDQ_TESTING_TRANSMIT,
	WDQ_TESTING_CARRIER,
	WDQ_TESTING_BOUNDARY
} wdq_testModes_e;

// ------------------------------------------------------------------------------------------
// Flags used to change the behavior of some API calls.  
// -- Not all API's with a wdq_apiFlags_e argument will support all flags.  See each API 
//    function description for details.
//
// Note: this is a bit field -- multiple flags may be set simultaneously
// ------------------------------------------------------------------------------------------
typedef enum {

	WDQ_API_NONE = 0,
	
	// Used when setting configuration options, means fall back to existing settings if the 
	// current connection is lost and cannot be restored. 	
	WDQ_API_ONLYIFCONNECT    = 1<<0,	
	
	// Used when setting path locations, means create the path if it does not exist
	WDQ_API_CREATEIFNOTEXIST = 1<<1,	
	
	// ??? What is this for? 
   WDQ_API_CHANGEPASSWORD   = 1<<2,	
   
   // For config queries.  Means fill structures with default configutation settings.
   WDQ_API_GETDEFAULTS      = 1<<3,	   
   
   // Got network element changes and queries, omit passwords and WEP keys.
	WDQ_API_NOPRIVATEDATA    = 1<<4,      
   
	WDQ_API_BOUNDARY = 1<<5,   
}
wdq_apiFlags_e;

// ------------------------------------------------------------------------------------------
// Flags that indicate support for certain features that may not be supported in all
// versions and/or deployments of the API.
//  
// Note: this is a bit field -- multiple flags may be set simultaneously
// ------------------------------------------------------------------------------------------
typedef enum {
	WDQ_VFLG_NONE = 0,
	WDQ_VFLG_CFG_BAND                 = 1<<0,		
	WDQ_VFLG_CFG_ANTENNA              = 1<<1,		
	WDQ_VFLG_DRIVER_LOGGING           = 1<<2,		
	WDQ_VFLG_CFG_CHANNEL              = 1<<3,		
	WDQ_VFLG_CFG_DEF_ADHOC_CHAN       = 1<<4,		
	WDQ_VFLG_CFG_NULL_PACKET_TIME     = 1<<5,		
	WDQ_VFLG_CFG_DISCONNECT_SCAN_TIME = 1<<6,		
	WDQ_VFLG_CFG_ROAMING_PARAMS   	 = 1<<7,		
}
wdq_versionFlags_e;

// ------------------------------------------------------------------------------------------
// Flags that indicate boolean values in scan elements
// ------------------------------------------------------------------------------------------
typedef enum {
	WDQ_SEOPTS_NONE = 0,
	WDQ_SEOPTS_ADHOC       = 1<<0,	 // 0-Infrastructure, 1-AdHoc 	
	WDQ_SEOPTS_CFG_EXISTS  = 1<<1,	 // 1-SSID exists in network store list (configured)
}
wdq_scanElemOpts_e;

// ------------------------------------------------------------------------------------------
// Ping test status values.
// ------------------------------------------------------------------------------------------
typedef enum  {
   WDQ_PINGRES_FINISHED  =  1,      
   WDQ_PINGRES_REPLYRECV =  0,
   WDQ_PINGRES_TIMEOUT   = -1,   
   WDQ_PINGRES_ERROR     = -2,      
}
wdq_pingResultsStatus_e;

//
// ------------------------------------------------------------------------------------------
// 
// API Structures
//
// ------------------------------------------------------------------------------------------
//

// WEP Keys
typedef struct _wepkeyStore {
	int           txKey;
	int           keylen[WDQ_NUM_WEP_KEYS_MAX];
	wdq_Byte_t    key   [WDQ_NUM_WEP_KEYS_MAX][WDQ_WEP_KEY_SIZE];	
} wdq_wepKeyStore_t ;

// ----------------------------------------------------------------------------
// Password Store Element 
//
// -- security config for a network
// ----------------------------------------------------------------------------

// Passwords and keys (private data)
typedef struct {
   wdq_wepKeyStore_t    wepKeys;
   wdq_Ascii_t          password    [WDQ_MAX_PASSWORD_SIZE+1]; // Could be saved EAP password or PSK
   wdq_Ascii_t          keyFilePwd  [WDQ_MAX_PASSWORD_SIZE+1]; // PAC or certificate file password   
} wdq_pwdStorePriv_t ;  

// Authentication configuration (not private)
typedef struct {
   wdq_authType80211_e  authType80211;        // 802.11 authentication
   wdq_passwordType_e   passwordType;         // Security type
   wdq_wpaType_e        wpaVersion;           // WPA1 or WPA2 or both
   wdq_eapType_e        eapMethods;           // One or more EAP types   
   wdq_eapOpts_e        eapOptions;           // One or more EAP options   
   wdq_authHint_e       authHints;            // One or more authentication hints
   wdq_Ascii_t          userIdentity[WDQ_MAX_NAME_SIZE+1];     // For authentication login.
   //wdq_Char_t           keyFile     [WDQ_CHBUF_SIZE(WDQ_MAX_PATH+1)]; // PAC or certificate file name.  (May include full path.)
   wdq_Char_t           keyFile     [WDQ_MAX_PATH+1]; // PAC or certificate file name.  (May include full path.)
   wdq_pwdStorePriv_t   priv;                 // stored encrypted
} wdq_pwdStoreElem_t ;
   
// ----------------------------------------------------------------------------
// Network Store Element
//
// - one of these for each item in the top-level network list
// ----------------------------------------------------------------------------
typedef struct {
   wdq_netElemOpts_e     options;
   int                   order;                    // 0-based 
   wdq_pmkCacheType_e    pmkCacheType;
   wdq_SSID_t            SSID;
   wdq_networkName_t     name;                     // matches name in network list
   wdq_pwdStoreElem_t    pwdInfo;                  // password store element
} wdq_netStoreElem_t ;

// ----------------------------------------------------------------------------
// Network Store 
//
// -- top level element of the networks configuration
// -- networkList contains the names of all configured networks
// ----------------------------------------------------------------------------
typedef struct {
   wdq_Bool_t         autoSwitch;  // true: use any network in the list
   wdq_networkName_t  currentNetwork; 
   int                networkCount;
   wdq_networkName_t  networkList[WDQ_MAX_NUM_NETWORKS];   
   wdq_Char_t         certStorePath[WDQ_CHBUF_SIZE(WDQ_MAX_PATH+1)];
} wdq_networkStore_t ;

// ----------------------------------------------------------------------------
// Radio configuration options
//
// -- these setting are global to all networks
// ----------------------------------------------------------------------------
typedef struct _radio_store {
   wdq_Char_t           clientName[WDQ_CHBUF_SIZE(WDQ_MAX_NAME_SIZE+1)];
   wdq_radioOption_e    options;
   int                  txPowerQdbm;         // -1=max, otherwise in units of quarter-dBm
   wdq_powerSaveMode_e  powerSaveMode;
   wdq_bandSelect_e	   bands;
   unsigned int         bgChanMask;
   unsigned int         bitRateMask;
   wdq_dfsChanMode_e    dfsChanMode;       
   int                  roamMagicFactor;     // (1-10) a stickyness factor to control roaming
   int                  adhocChannel;        // it will start the cell here if no matching adhoc network found
   wdq_IPaddr_t         remoteServerIP; 
   wdq_testModes_e      testMode; 
   wdq_debugType_e      debugMode;   
   wdq_antennaSelect_e  antennaConfig;
   int                  nullPacketTimeS;     // # of seconds of no traffic before a null packet is sent
   int                  disconnectScanTime;  // time between scans when disconnected
   int                  fragThresh;
   int                  rtsThresh;   
} wdq_radioStore_t ;


// ----------------------------------------------------------------------------
//    *** TBD ***
// ----------------------------------------------------------------------------
typedef struct {
	int dummy1;
} wdq_radioSpecificStore_t ;
  
// ----------------------------------------------------------------------------  
// Radio Data
//
// -- all configuration data, including networks and radio options
// ----------------------------------------------------------------------------  

typedef struct {
	wdq_networkStore_t       networks;
	wdq_radioStore_t         radioStore;
	wdq_radioSpecificStore_t radioSpecificData;
} wdq_radioData_t ;


// ----------------------------------------------------------------------------  
//
// Status
//
// -- all available status information: connection and radio
// -- readonly
//
// ----------------------------------------------------------------------------  

typedef struct {

	unsigned int      verDriver;
	unsigned int      verSupplicant;
   
	wdq_radioType_e   adapterType; 
   wdq_regDomain_e   regDomain;
	wdq_Byte_t        MAC  [WDQ_MAC_ADDR_SIZE];
	wdq_Byte_t        MACAP[WDQ_MAC_ADDR_SIZE];

	wdq_IPaddr_t      IPaddr;
	wdq_IPaddr_t      IPaddrAP;	
	wdq_IPaddr_t      IPaddrRCServer; 

   wdq_SSID_t        SSID;

   wdq_connStatus_e  connStatus;	
	int               channelNum;   
	int               RSSI;  
	int               SNR;
	int               bitRate;
	
	unsigned int      framesTx;
	unsigned int      framesRx;
	unsigned int       bytesTx;
	unsigned int       bytesRx;	

   wdq_Bool_t        adapterReady;
   wdq_Bool_t        probing;          // if probe sent this will be set til its completed
} 
wdq_status_t;


// ----------------------------------------------------------------------------  
// AP Scan List 
// ----------------------------------------------------------------------------  

typedef struct {
   wdq_SSID_t         SSID;
	wdq_Byte_t         MAC[WDQ_MAC_ADDR_SIZE];
   wdq_passwordType_e netType;
   wdq_scanElemOpts_e options;
   wdq_bandSelect_e	 band;
   int                channel;   
   int                RSSI;   
   int                bitRateKHz;   // maximum bit rate supported, in KHz
} wdq_scanElement_t ;

typedef struct {
	int               elemCount;
	wdq_scanElement_t elemList[ WDQ_MAX_PROBE_AP ];
} wdq_scanList_t ;


// ----------------------------------------------------------------------------------------
// Ping Test results structure
// ----------------------------------------------------------------------------------------

// Used to report cumulative results of a ping test.
typedef enum {
   WDQ_PINGSTAT_FINISHED   =  1,      
   WDQ_PING_STAT_REPLYRECV =  0,
   WDQ_PING_STAT_TIMEOUT   = -1,   
   WDQ_PING_STAT_ERROR     = -2,      
} wdq_pingStatus_e;

typedef struct {
   int status;    // status of last ping request
   int error;     // an error code if status is PING_STATUS_ERROR 
   int lost;      // number of echo requests that failed or timed out
   int recv;      // number of echo replies received
   int timeLast;  // ms, rount-trip time of last echo request/reply
   int timeTotal; // ms, total time up through last repl received   
   int timeMin;   // ms, minimum rount-trip time
   int timeMax;   // ms, maximum round-trip time
} wdq_pingResults_t;


// ----------------------------------------------------------------------------  
//
// API Function prototypes 
//
// ----------------------------------------------------------------------------  


// ----------------------------------------------------------------------------------------
// Returns the version number of the API.  
//   Format is Major.miNor.Revision with the digits assigned MMNNRRRR
//   as in the following examples: 1010010 = 1.01.0010, 12345678 = 12.34.5678
//
// Args:
// - verFlags: if not NULL, receives the version flags (may be NULL) 
// ----------------------------------------------------------------------------------------
//UINT DllExport WdqGetVersion( UINT reserved, wdq_versionFlags_e *verFlags );
UINT DllExport WdqGetVersion( UINT reserved );

// Macros for building and parsing out the pieces of the version number
#define WDQ_VERSION(maj,min,rev)  ((((maj)&0xff)<<24) + (((min)&0xff)<<16) + ((rev)&0xffff))

#define WDQ_VER_MAJ(v)  ((v)>>24)
#define WDQ_VER_MIN(v) (((v)>>16)&0xff)
#define WDQ_VER_REV(v)  ((v)&0xffff)

// ----------------------------------------------------------------------------------------
// Retrieves all configuration data.
// - Always returns success.  Missing configuration settings are given default values.
//
// - flags: WDQ_API_GETDEFAULTS: the data structure is filled with the configuration 
//                               default settings. 
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqGetConfiguration( wdq_radioData_t *data, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Retrieves the current connection and radio status
// - Always returns success.  Missing configuration settings are given default values.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqGetStatus( wdq_status_t *status, UINT reserved);

// ----------------------------------------------------------------------------------------
// Sets the name of the network to which to connect. 
//
// Arguments:
// - name : an existing, configured network 
// - flags: WDQ_API_ONLYIFCONNECT
//
// Errors: 
// - WDQ_APIERR_NOTFOUND: if name is not found in the network list
// - WDQ_APIERR_BADARG  : if flags value is not supported
// - WDQ_APIERR_BADWRITE: if attempt to save the setting fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqSetCurrentNetwork( const wdq_Char_t *name, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Retrieves the configuration settings of an existing network.
//
// Arguments:
// - name: an existing, configured network 
// - elem: pointer to network element buffer to receive the network settings.
// - flags : 
//   -- WDQ_API_GETDEFAULTS: the elem structure is filled with the configuration default 
//                           settings.  The 'name' arg is ignored and may be NULL. 
//   -- WDQ_API_NOPRIVATEDATA: if set, doesn't read the private data(passwords/keys)
//
// Errors: 
// - WDQ_APIERR_NOTFOUND: if name is not found in the network list
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqGetNetwork( const wdq_Char_t *name, wdq_netStoreElem_t *elem, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Adds a new network element.
//
// Errors:
// - WDQ_APIERR_COLLISION: if a network with the same name already exists
// - WDQ_APIERR_BADARG   : if flags value is not supported
// - WDQ_APIERR_BADWRITE : if attempt to save the setting fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqAddNetwork( wdq_netStoreElem_t *elem, UINT reserved );

// ----------------------------------------------------------------------------------------
// Sets the contents of an existing network element to the contents of the provided network 
// element structure.
//
// Arguments:
// - wdqNet: settings to replace existing network element with the same name
// - flags:
//   -- WDQ_API_ONLYIFCONNECT: if the network to be changed is the current network,
//        revert back to the old settings of the connection is lost and cannot be restored.
//   -- WDQ_API_NOPRIVATEDATA: if set, ignores private data fields (password/keys)
//  
// Errors:
// - WDQ_APIERR_NOTFOUND: if wdqNet->name is not found in the network list
// - WDQ_APIERR_BADWRITE: if attempt to save the settings fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqChangeNetwork( const wdq_Char_t *name, wdq_netStoreElem_t *elem, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Deletes an existing network element with the given name.
//
// Arguments:
// - name : an existing, configured network 
// - flags: WDQ_API_ONLYIFCONNECT: if a connection can not be restored after deleting 
//          the network, restore the network settings and reconnect to it.
//
// Errors:
// - WDQ_APIERR_NULLPTR : if name is NULL
// - WDQ_APIERR_NOTFOUND: if name is not found in the network list
// - WDQ_APIERR_BADWRITE: if attempt to delete the network settings fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqDeleteNetwork( const wdq_Char_t *name, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Chooses whether the wi-daq supplicant is allowed to switch profiles when necessary
// to establish a connection.  
//  
// Arguments:
// - enable: TRUE (Enabled): may switch to other saved network profiles if a connection 
//                           cannot be established to the current network
//           FALSE (Disabled): always uses the current network setting  (never switches)
//
// Errors: WDQ_APIERR_BADWRITE: if attempt to save the setting fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqEnableAutoNetworkSwitch( wdq_Bool_t enable, UINT reserved );

// ----------------------------------------------------------------------------------------
// Set one or more radio configuration options.
//
// NOTE: First read the current settings. Then replace one or more of the returned settings
//       with new values and write the settings back.  See the following example code:
//
//       Example: wdq_radioStore_t store;
//                WdqGetRadioParameters( &store, 0 );
//                store.autoConnect = true;
//                WdqSetRadioParameters( &store, 0 );
//
// Arguments:
// - wdqRadio: pointer to buffer containing new radio settings
// - flags   : WDQ_API_ONLYIFCONNECT: if connection is lost and cannot be restored, fall 
//             back to the original settings
//
// Errors:
// - WDQ_APIERR_BADARG  : if flag value is not supported
// - WDQ_APIERR_BADWRITE: if attempt to save the settings fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqSetRadioParameters( const wdq_radioStore_t *radio, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Get or set the path to the currently configured certificate directory
//
// Arguments:
// - flags: WDQ_API_CREATEIFNOTEXIST create the directory if it does not exist (Set only)
//
// Errors:
// - WDQ_APIERR_NOTFOUND : if name is not found in the network list
// - WDQ_APIERR_BADWRITE : if attempt to save the path fails
// - WDQ_APIERR_BADCREATE: if attempt to create the path fails
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqSetCertPath( const wdq_Char_t *certPath, wdq_apiFlags_e flags );

// ----------------------------------------------------------------------------------------
// Requests that the driver refresh its list of nearby access points.
// -- this function returns immediately, but it may take a few seconds before the driver
//    finishes its scan
//
// Errors: WDQ_APIERR_GENERAL: if driver is not ready
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqStartScan( UINT reserved );

// ----------------------------------------------------------------------------------------
// Retrieve the driver's most recent list of nearby access points (APs)
//
// Errors: 
// - WDQ_APIERR_GENERAL : if driver is not ready
// - WDQ_APIERR_NOTFOUND: if most recent requested scan has not yet completed.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqGetScanList( wdq_scanList_t *pList, UINT reserved );

// ----------------------------------------------------------------------------------------
//
// Pass-through to allow direct access to driver OID calls
//
// ----------------------------------------------------------------------------------------
//WDQAPIERR DllExport WdqGetOid(unsigned int oid, void *data, unsigned int *dataSize, UINT reserved);
//WDQAPIERR DllExport WdqSetOid(unsigned int oid, void *data, unsigned int  dataSize, UINT reserved);

#ifdef _SWIG
#define STDCALL
#else
#define STDCALL __stdcall
#endif

// ----------------------------------------------------------------------------------------
// Starts a ping test.
// - caller provides a callback function to receive results
// ----------------------------------------------------------------------------------------
typedef void (STDCALL * wdq_fnPingCallback_t) ( wdq_pingResults_t *pResults, UINT reserved ); 
WDQAPIERR DllExport WdqStartPingTest( const wdq_Char_t *ipAddr, int count, wdq_fnPingCallback_t pfn, UINT reserved ); 

// ----------------------------------------------------------------------------------------
// Stops a ping test if one is in progress. 
// ----------------------------------------------------------------------------------------
void DllExport WdqStopPingTest( UINT reserved );

// ----------------------------------------------------------------------------------------
// Returns cumulative results of most recent ping test.  
// -- May be called repeatedly while a test is in progress, as well as after a test has
//    stopped.  Always succeeds: if no test has been run returns zeroed results.
// ----------------------------------------------------------------------------------------
//WDQAPIERR DllExport WdqGetPingResults( wdq_pingResults_t *p );

// ----------------------------------------------------------------------------------------
// Enables and disables connection activity logging.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqEnableLogging( wdq_Bool_t bEnable, wdq_debugType_e dbgType, UINT reserved );

// ----------------------------------------------------------------------------------------
// Redirects API debug tracing to the caller's print function.  (This output will 
// consist mainly of error messages related to API calls.)
// 
// Arguments:
// -- pfnDbgOut: pointer to a function to be called whenever the API has a tracing message
//               to ouput.  Set to NULL to disable again.
// ----------------------------------------------------------------------------------------
typedef void (STDCALL * wdq_fnDbgOut_t) ( const wdq_Char_t *psz ); 
void DllExport WdqSetDbgOut( const wdq_fnDbgOut_t pFunc, UINT reserved );


/*
These are implemented in WIM

// ----------------------------------------------------------------------------------------
// Unlocks the configuration data, which allows changes to be made.  
// - returns WDQ_APIERR_BADARG, if password is incorrect.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqUnlockConfig( wdq_Char_t *password, UINT reserved );

// ----------------------------------------------------------------------------------------
// Locks the configuration data.  
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqLockConfig( UINT reserved );

// ----------------------------------------------------------------------------------------
// Changes the password used to unlock the configuration data.
// - returns WDQ_APIERR_BADARG, if oldPassword is incorrect.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqChangeAdminPassword( wdq_Char_t *oldPassword, wdq_Char_t *newPassword, UINT reserved );
*/



/*
Don't need this.  WIM treats the PSK like a string.

// ----------------------------------------------------------------------------------------
// Generates a WPA PSK key from a passphrase and an SSID.
// - returns WDQ_APIERR_BADARG if passphrase text is incorrectly formatted.
//   Valid formats are: a 8-63 char string or a 64 digit hex string
// ----------------------------------------------------------------------------------------
//WDQAPIERR DllExport WdqGenWpaPsk( const char *text, const wdq_SSID_t *ssid, wdq_Byte_t *key, UINT reserved );
*/


#ifdef __cplusplus
}
#endif
#endif // WDQAPI_H
