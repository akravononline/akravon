#ifdef __cplusplus
extern "C" {
#endif

    /* TEA is a 64-bit symmetric block cipher with a 128-bit key, developed
       by David J. Wheeler and Roger M. Needham, and described in their
       paper at <URL:http://www.cl.cam.ac.uk/ftp/users/djw3/tea.ps>.

       This implementation is based on their code in
       <URL:http://www.cl.cam.ac.uk/ftp/users/djw3/xtea.ps> */

	#define TEA_KEY_LENGTH 16

    // Cynex: 64bit'te unsigned long 8 bayt - blok 32bit kalmali (windows'ta DWORD = unsigned long)
    int tea_encrypt(DWORD *dest, const DWORD *src, const DWORD *key, int size);
    int tea_decrypt(DWORD *dest, const DWORD *src, const DWORD *key, int size);

#ifdef __cplusplus
};
#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
