#include <windows.h>
#define MaxDim 128

int X = 80, Y = 60, XY = 4800; // dimensions: XY = X*Y
unsigned char cel[MaxDim*MaxDim], cpy[MaxDim*MaxDim]; // algorithme
char *str; // presse-papier
int idx; // index dans str
HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); // console
WORD att[MaxDim*MaxDim], vive = 192, mort = 0; // console

void Etape() {
  int X1 = X-1, Y1 = Y-1;
  unsigned char n, *c = cel, *p, *a = cpy, *s = cpy+X;
  memcpy(cpy, cel, XY); // dédoublement
  for (int y = 1; y < Y1; ++y) { // évite les bords
    c += X; p = a; a = s; s += X; // incrémentation des pointeurs
    for (int x = 1; x < X1; ++x) { // évite les bords
      n = p[x-1]+p[x]+p[x+1]+a[x-1]+a[x+1]+s[x-1]+s[x]+s[x+1];
      if (n != 2) // évolution selon le nombre de voisins vivants n
        if (c[x]) {if (n != 3) c[x] = 0;}
        else {if (n == 3) c[x] = 1;}
    } // for (x)
  } // for (y)
}

int CardDec() { // lit l'entier positif décimal suivant dans str.
  while ((str[idx] < '0')||(str[idx] > '9')) // avance jusqu'au prochain chiffre
    if (str[idx]) ++idx; else return -1; // fin: -1
  int n = (str[idx++])-'0';
  while (('0' <= str[idx])&&(str[idx] <= '9')) n = 10*n + str[idx++] - '0';
  return n;
}

void PressePapier() {
  OpenClipboard(0);
  HANDLE hCD = GetClipboardData(CF_TEXT);
  str = (char*)GlobalLock(hCD);
  CloseClipboard();
  if ((str[0]!='J')||(str[1]!='V')||(str[2]!='a'))
    {MessageBox(0, "code 'JVa' ?", "Erreur Presse-papier", 0); XY = 0; return;}
  idx = 0; X = CardDec(); Y = CardDec(); XY = X*Y;
  if ((X > MaxDim)||(Y > MaxDim))
    {MessageBox(0, "Dimensions ?", "Erreur Presse-papier",0); XY = 0; return;}
  int k = 0, n, v = 1;
  while ((n = CardDec()) >= 0) {memset(cel+k, (v = 1-v), n); k += n;}
}

void Dimension() {
  BOOL b = SetConsoleScreenBufferSize(hOut, {X, Y});
  SMALL_RECT r = {0, 0, X-1, Y-1};
  SetConsoleWindowInfo(hOut, 1, &r);
  if ((b == 0)&&(SetConsoleScreenBufferSize(hOut, {X, Y}) == 0))
    {MessageBox(0,"Console Dimension","Erreur",0); return;}
}

void Visualisation() {
  for (int k = 1; k < XY; ++k) att[k] = cel[k] ? vive : mort;
  WriteConsoleOutputAttribute(hOut, att, XY, {0,0}, 0);
}

void main() {
  char help[] = "h: help (aide)\ni: initialisation depuis le presse-papier\n"
    "p: évoluer d'un pas\nd: évoluer de 10 pas\nc: évoluer de cent pas\n"
    "m: évoluer de mille pas\nclick: inverser une cellule";
  INPUT_RECORD inR[128];
  SetConsoleTitle("Jeu de la Vie   (h: Help, Aide)");
  CONSOLE_FONT_INFOEX fid = {sizeof(CONSOLE_FONT_INFOEX),0,{8,8},0,400,L"Terminal"};
  if (SetCurrentConsoleFontEx(hOut, 0, &fid) == 0)
    {MessageBox(0, "SetCurrentConsoleFont", "Erreur", 0); return;}
  Dimension();
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  CONSOLE_CURSOR_INFO ci = {1, FALSE}; 
  SetConsoleCursorInfo(hOut, &ci); 
  SetConsoleMode(hIn, ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
  while (1) {DWORD nIn;
    ReadConsoleInput(hIn, inR, 128, &nIn);
    for (int i = 0, k = 0; i < (int)nIn; ++i) {
      if (inR[i].EventType == KEY_EVENT) {
        KEY_EVENT_RECORD ker = inR[i].Event.KeyEvent;
        if (ker.bKeyDown)
          switch (ker.uChar.AsciiChar) {
          case 'h': MessageBox(0, help,"Jeu de la Vie: Help (Aide)",0); break;
          case 'i': PressePapier(); //initialisation depuis le presse-papier
            if (XY > 0) {Dimension(); k = 0; Visualisation();} break;
          case 'p': if (XY > 0) k = 1; break; // pas
          case 'd': if (XY > 0) k = 10; break; // dix pas
          case 'c': if (XY > 0) k = 100; break; // cent pas
          case 'm': if (XY > 0) k = 1000; break; // mille pas
          }
        while (k > 0) {k--; Etape(); Visualisation(); Sleep(50);}
      } else if (inR[i].EventType == MOUSE_EVENT) {
        MOUSE_EVENT_RECORD mer = inR[i].Event.MouseEvent;
        if ((mer.dwEventFlags == 0)&&(mer.dwButtonState == 1)) {
          int k = mer.dwMousePosition.X + X*mer.dwMousePosition.Y;
          cel[k] = 1-cel[k]; Visualisation();
        }
      }
    } // for (i)
  } // while (1)
}