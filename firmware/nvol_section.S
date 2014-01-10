/* flashをEEPROMとして使うため、プログラムのないダミー領域を確保する。flashが2ページ分必要。
   for lpc11xx
*/
.section ".rodata"
.balign 0x1000                  /* flashのページ境界にアライン */
.global flash_nvol_section1
flash_nvol_section1:
.incbin "flash_nvol_dummy.dat", 0, 0x1000  /* flashのデフォルト値である0xFFが書き込まれた4kBのファイル */
.global _sizeof_flash_nvol_section1
.set _sizeof_flash_nvol_section1, . - flash_nvol_section1

.section ".rodata"
.balign 0x1000
.global flash_nvol_section2
flash_nvol_section2:
.incbin "flash_nvol_dummy.dat", 0, 0x1000
.global _sizeof_flash_nvol_section2
.set _sizeof_flash_nvol_section2, . - flash_nvol_section2

.section "text"
