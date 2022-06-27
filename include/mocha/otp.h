
#include "mocha.h"
#include <stdint.h>
#include <assert.h>

// Ensure structs are correct size & offsets
#if defined(static_assert) || defined(__cplusplus)
#  define MOCHA_CHECK_SIZE(Type, Size) \
      static_assert(sizeof(Type) == Size, \
                    #Type " must be " #Size " bytes")

#  define MOCHA_CHECK_OFFSET(Type, Offset, Field) \
      static_assert(offsetof(Type, Field) == Offset, \
                    #Type "::" #Field " must be at offset " #Offset)
#else
#  define MOCHA_CHECK_SIZE(Type, Size)
#  define MOCHA_CHECK_OFFSET(Type, Offset, Field)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum OTPSecurityLevel {
    // Combined flags as found in consoles
    SECURITY_RETAIL_STATE                = 0x90000000,
    SECURITY_DEBUG_STATE                 = 0x88000000,
    SECURITY_FACTORY_STATE               = 0x00000000,
    // Flags
    SECURITY_FLAG_UNKNOWN                = 0x40000000, // Unknown, causes error in boot0
    SECURITY_FLAG_CONSOLE_PROGRAMMED     = 0x80000000, // Console type has been programmed
    SECURITY_FLAG_USE_DEBUG_KEY_IMAGE    = 0x08000000, // Use first RSA key and debug ancast images in boot0
    SECURITY_FLAG_USE_RETAIL_KEY_IMAGE   = 0x10000000  // Use second RSA key and retail ancast images in boot0
} OTPSecurityLevel;

typedef enum OTPIOStrength {
    IO_HW_IOSTRCTRL0   = 0x00008000,
    IO_HW_IOSTRCTRL1_3 = 0x00002000,
    IO_HW_IOSTRCTRL1_2 = 0x00000800,
    IO_HW_IOSTRCTRL1_1 = 0x00000080,
    IO_HW_IOSTRCTRL1_0 = 0x00000008,
    IO_NONE            = 0x00000000
} OTPIOStrength;

typedef enum OTPPulseLength {
    PULSE_BOOT0     = 0x0000002F,
    PULSE_NONE      = 0x00000000
} OTPPulseLength;

typedef enum OTPJtagStatus {
    JTAG_RETAIL_DEFAULT = 0xE1,
    JTAG_DISABLED       = 0x80
} OTPJtagStatus;

typedef struct OTPWiiBank {
    uint8_t boot1SHA1Hash[0x14];
    uint8_t commonKey[0x10];
    uint32_t ngId;
    uint8_t ngPrivateKey[0x1C];
    uint8_t nandHMAC[0x14];
    uint8_t nandKey[0x10];
    uint8_t rngKey[0x10];
    uint8_t unknownOrPadding[0x08];
} OTPWiiBank;
MOCHA_CHECK_SIZE(OTPWiiBank, 0x80);

typedef struct OTPWiiUBank {
    OTPSecurityLevel securityLevel;
    OTPIOStrength ioStrength;
    OTPPulseLength pulseLength;
    uint32_t signature;
    uint8_t starbuckAncastKey[0x10];
    uint8_t seepromKey[0x10];
    uint8_t unknown1[0x10];
    uint8_t unknown2[0x10];
    uint8_t vWiiCommonKey[0x10];
    uint8_t wiiUCommonKey[0x10];
    uint8_t unknown3[0x10];
    uint8_t unknown4[0x10];
    uint8_t unknown5[0x10];
    uint8_t sslRSAKey[0x10];
    uint8_t usbStorageSeedsKey[0x10];
    uint8_t unknown6[0x10];
    uint8_t xorKey[0x10];
    uint8_t rngKey[0x10];
    uint8_t slcKey[0x10];
    uint8_t mlcKey[0x10];
    uint8_t sshdKey[0x10];
    uint8_t drhWLAN[0x10];
    uint8_t unknown7[0x30];
    uint8_t slcHmac[0x14];
    uint8_t unknown8[0x0C];
} OTPWiiUBank;
MOCHA_CHECK_SIZE(OTPWiiUBank, 0x180);

typedef struct OTPWiiUNGBank {
    uint8_t unknown1[0x10];
    uint8_t unknown2[0x0C];
    uint32_t ngId;
    uint8_t ngPrivateKey[0x20];
    uint8_t privateNSSDeviceCertKey[0x20];
    uint8_t otpRNGSeed[0x10];
    uint8_t unknown3[0x10];
} OTPWiiUNGBank;
MOCHA_CHECK_SIZE(OTPWiiUNGBank, 0x80);


typedef struct OTPWiiUCertBank {
    uint32_t rootCertMSId;
    uint32_t rootCertCAId;
    uint32_t rootCertNGKeyId;
    uint8_t rootCertNGSignature[0x3C];
    uint8_t unknown1[0x18];
    uint8_t unknown2_protected[0x20];
} OTPWiiUCertBank;
MOCHA_CHECK_SIZE(OTPWiiUCertBank, 0x80);

typedef struct OTPWiiCertBank {
    uint32_t rootCertMSId;
    uint32_t rootCertCAId;
    uint32_t rootCertNGKeyId;
    uint8_t rootCertNGSignature[0x3C];
    uint8_t koreanKey[0x10];
    uint8_t unknown1[0x08];
    uint8_t privateNSSDeviceCertKey[0x20];
} OTPWiiCertBank;
MOCHA_CHECK_SIZE(OTPWiiCertBank, 0x80);

typedef struct OTPMiscBank {
    uint8_t unknown1_protected[0x20];
    uint8_t boot1Key_protected[0x10];
    uint8_t unknown2_protected[0x10];
    uint8_t padding1[0x20];
    uint8_t padding2[0x04];
    uint32_t otpVersionAndRevision;
    uint64_t otpDateCode;
    char otpVersionName[0x08];
    uint8_t padding3[0x04];
    OTPJtagStatus jtagStatus;
} OTPMiscBank;
MOCHA_CHECK_SIZE(OTPMiscBank, 0x80);

typedef struct WiiUConsoleOTP {
    OTPWiiBank wiiBank;
    OTPWiiUBank wiiUBank;
    OTPWiiUNGBank wiiUNGBank;
    OTPWiiUCertBank wiiUCertBank;
    OTPWiiCertBank wiiCertBank;
    OTPMiscBank miscBank;
} WiiUConsoleOTP;
MOCHA_CHECK_SIZE(WiiUConsoleOTP, 0x400);

/**
 * Read the consoles OTP into the given buffer.
 *
 * @param out_buffer Buffer where the result will be stored.
 * @return MOCHA_RESULT_SUCCESS: The OTP has been read into the buffer<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: invalid environmentPathBuffer pointer or bufferLen \< 0x100<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_ReadOTP(WiiUConsoleOTP *out_buffer);

#ifdef __cplusplus
} // extern "C"
#endif