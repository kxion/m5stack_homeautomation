#ifndef DUNK__GOOGLESHEET__CONFIG_H
#define DUNK__GOOGLESHEET__CONFIG_H

#define WEB_SERVER "script.google.com"
#define WEB_PORT "443"
#define WEB_URL "https://script.google.com/macros/s/AKfycbxnv6e2OrhUPW0lw05jHF-sX64VJRyBf1HdgUxjxdtR18rWHVA/exec"

void https();
uint8_t googleSheet_connect(const char* url);

#endif  // DUNK__GOOGLESHEET__CONFIG_H
