byte x2b(char c) {
  if (isdigit(c)) {  // 0 - 9
    return c - '0';
  } 
  else if (isxdigit(c)) { // A-F, a-f
    return (c & 0xF) + 9;
  }

}

long convert_rgb(char* rgb_str) {
    long rgb = 0;

    for (int i= 0; i < strlen(rgb_str); i++) {
        rgb = (rgb * 16) + x2b(rgb_str[i]);
    }

    return rgb;
}