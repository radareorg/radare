/***
 *  Z80 Disassembler
 *
 *  Dieser kleine Disassembler für Z80-Code ist an einem Nachmittag entstanden.
 *  Es gibt keine Benutzeroberfläche! Größe des zu disassemblierenden ROMs und
 *  eventuelle weitere Sprünge sind direkt im Programmcode zu ändern!!!
 *
 *  Er läßt sich unter Think C 5.0 auf dem Macintosh übersetzen. Wer keinen
 *  Macintosh hat, darf zum einen die Umlaute im Sourcecode wandeln, und
 *  — sofern man keinen C++ Compiler hat — die Kommentare von \\ umstellen.
 *
 *  Ferner wird eine ANSI-Library (Dateifunktionen) benötigt. Aber auch das
 *  kann man leicht umstellen (siehe main()). Hier wird stets die Datei “EPROM”
 *  geladen. Kann man natürlich ändern…
 *
 *  Das Programm besteht aus zwei Teilen:
 *  1.  Analyse des Programms. Hierbei wird das Programm ab den verschiedenen
 *      Hardware-Vektoren des Z80 (RST-Befehle, NMI) durchgegangen und alle Spünge
 *      durch ein rekursives Unterprogramm (ParseOpcodes) ausgeführt. Dabei werden
 *      gefundene Opcodes in einem Array (OpcodesFlags) markiert. Auch Adressen, die
 *      als Sprungziele verwendet werden, sind dort markiert. Der Disassembler
 *      kann später also genau erkennen, ob er Daten oder Programmcode vor sich hat!
 *      Dabei gibt es natürlich Ausnahmen, die er nicht erkennen kann:
 *      a)  selbstmodifizierender Code. Sowas sollte in einem ROM normalerweise
 *          nicht auftreten.
 *      b)  berechnete Sprünge mit JP (IY), JP (IX) oder JP (HL). Auch hier kann
 *          der Parser die Sprünge nicht erkennen. Man landet im MacsBug, wenn
 *          so ein Sprung gefunden wurde. Wer das Symbol DEBUGGER auf 0 setzt,
 *          hat Ruhe…
 *      c)  Sprungtabellen. Diese treten leider recht häufig auf. Einzige Lösung:
 *          Programm disassemblieren und ansehen. Wenn man die Sprungtabellen
 *          gefunden hat, kann man — wie bei meinem Futura Aquariencomputer ROMs
 *          geschehen — weitere ParseOpcodes(Opcodes, ) Aufrufe einfügen. Wie und wo das
 *          geht, steht in main()
 *      d)  Unbenutzer Code. Code der nie angesprungen wird, wird natürlich durch
 *          die Analyse nicht gefunden. I.d.R. ist es um solchen Code auch nicht
 *          schade :-) Häufig wird der "unbenutzte" Code jedoch über eine Sprungtabelle
 *          angesprungen! Also Achtung!
 *  2.  Disassemblieren des Programms. Mit Hilfe der beim Parsen erzeugten
 *      OpcodesFlags-Tabelle wird nun ein Listing erzeugt. Das Disassemble-Unterprogramm
 *      ist leider recht "länglich". Es disassembliert einen Opcode ab einer Adresse
 *      im ROM in einen Buffer. Ich habe es in einem Stück runtergeschrieben (an Hand
 *      einer Opcodeliste). Inbesondere die Verwaltung von IX und IY kann man sicher
 *      stark kürzen…
 *
 *  Das Unterprogramm z80_OpcodeLen() ermittelt die Länge eines Opcodes in Bytes. Es
 *  wird während des Parsens und während des Disassemblierens benötigt.
 *
 *  Der Disassembler kennt übrigens _keine_ versteckten Opcodes des Z80. Ich hatte
 *  keine Tabelle darüber. In meinem Fall waren die auch gar nicht nötig… Wer so
 *  eine Liste hat, kann den Disassembler ja mal ergänzen.
 *
 *  Wenn übrigens ein Unterprogramm eine "Adresse" im Z80-Code erwartet, so ist
 *  damit ein _Offset_ auf das Array mit dem Code gemeint! Pointer sind es NICHT!
 *  Longs sind übrigens unnötig, denn ein Z80 hat ja nur 64K…
 *
 *  In main() kann man anstatt einer Disassemblierung mit Labeln auch eine mit
 *  Adresse und Hexdump vor dem Opcode einstellen. Sehr praktisch um evtl. Fehler
 *  im Disassembler zu finden oder beim Erstellen einer Variablenliste.
 *
 *
 *  Das Programm ist Freeware. Es darf _nicht_ als Basis für ein kommerzielles
 *  Produkt genommen werden! Ich übernehme keine Haftung für Schäden, die direkt
 *  oder indirekt durch die Benutzung dieses Programms entstehen!
 *
 *  Wer mich erreichen will, kann dies am besten in unserer Firmen-Mailbox:
 *
 *  Sigma-Soft-Mailbox
 *  ©1992 ∑-Soft, Markus Fritze
 ***/
 
#include <stdio.h>
#include <string.h>
#include "portab.h"
 
#define CODESIZE        8192L           // 8K Programmcode
#define FUTURA_189      1               // Sprungtabellen-Sprünge für Futura Aquariencomputer ROM V1.89
#define DEBUGGER        0               // wenn 1, dann landet man bei berechneten
                                        // Sprüngen im Debugger. Siehe auch oben.
 
// Speicher für den Programmcode
//UBYTE       Opcodes[CODESIZE];
 
// Flag pro Speicherstelle, ob Opcode, Operand, Daten
// Bit 4 = 1, d.h. hier wird per JR o.ä. hingesprungen.
enum {
    Opcode,
    Operand,
    Data
} DataType;
 
UBYTE       OpcodesFlags[CODESIZE];
 
// Länge eines Opcodes in Bytes ermitteln
UBYTE z80_OpcodeLen(UBYTE* Opcodes, ULONG p, int *type)
{
	UBYTE   len = 1;
	int type_tmp;
	if (type == NULL)
		type = &type_tmp;

    switch(Opcodes[p]) {// Opcode
    case 0x10:          // DJNZ e
    case 0x18:          // JR e
    case 0x20:          // JR NZ,e
    case 0x28:          // JR Z,e
    case 0x30:          // JR NC,e
    case 0x38:          // JR C,e
	*type = 'b';
	// len = 2.. so no break here
    case 0x06:          // LD B,n
    case 0x0E:          // LD C,n
    case 0x16:          // LD D,n
    case 0x1E:          // LD E,n
    case 0x26:          // LD H,n
    case 0x2E:          // LD L,n
    case 0x36:          // LD (HL),n
    case 0x3E:          // LD A,n
    case 0xC6:          // ADD A,n
    case 0xCE:          // ADC A,n
    case 0xD3:          // OUT (n),A
    case 0xD6:          // SUB n
    case 0xDB:          // IN A,(n)
    case 0xDE:          // SBC A,n
    case 0xE6:          // AND n
    case 0xEE:          // XOR n
    case 0xF6:          // OR n
    case 0xFE:          // CP n
    case 0xCB:          // Shift-,Rotate-,Bit-Befehle
                len = 2;
                break;
    case 0x01:          // LD BC,nn'
    case 0x11:          // LD DE,nn'
    case 0x21:          // LD HL,nn'
    case 0x22:          // LD (nn'),HL
    case 0x2A:          // LD HL,(nn')
    case 0x31:          // LD SP,(nn')
    case 0x32:          // LD (nn'),A
    case 0x3A:          // LD A,(nn')
		len = 3;
		break;
    case 0xC4:          // CALL NZ,nn'
    case 0xCC:          // CALL Z,nn'
    case 0xCD:          // CALL nn'
    case 0xD4:          // CALL NC,nn'
    case 0xDC:          // CALL C,nn'
    case 0xE4:          // CALL PO,nn'
    case 0xEC:          // CALL PE,nn'
    case 0xF4:          // CALL P,nn'
    case 0xFC:          // CALL M,nn'
		len = 3;
		*type = 'c';
		break;
    case 0xC2:          // JP NZ,nn'
    case 0xC3:          // JP nn'
    case 0xCA:          // JP Z,nn'
    case 0xD2:          // JP NC,nn'
    case 0xDA:          // JP C,nn'
    case 0xE2:          // JP PO,nn'
    case 0xEA:          // JP PE,nn'
    case 0xF2:          // JP P,nn'
    case 0xFA:          // JP M,nn'
// XXX: use call or jp for opcode type correctly
		*type = 'b';
                len = 3;
                break;
    case 0xDD:  len = 2;
                switch(Opcodes[p+1]) {// 2.Teil des Opcodes
                case 0x34:          // INC (IX+d)
                case 0x35:          // DEC (IX+d)
                case 0x46:          // LD B,(IX+d)
                case 0x4E:          // LD C,(IX+d)
                case 0x56:          // LD D,(IX+d)
                case 0x5E:          // LD E,(IX+d)
                case 0x66:          // LD H,(IX+d)
                case 0x6E:          // LD L,(IX+d)
                case 0x70:          // LD (IX+d),B
                case 0x71:          // LD (IX+d),C
                case 0x72:          // LD (IX+d),D
                case 0x73:          // LD (IX+d),E
                case 0x74:          // LD (IX+d),H
                case 0x75:          // LD (IX+d),L
                case 0x77:          // LD (IX+d),A
                case 0x7E:          // LD A,(IX+d)
                case 0x86:          // ADD A,(IX+d)
                case 0x8E:          // ADC A,(IX+d)
                case 0x96:          // SUB A,(IX+d)
                case 0x9E:          // SBC A,(IX+d)
                case 0xA6:          // AND (IX+d)
                case 0xAE:          // XOR (IX+d)
                case 0xB6:          // OR (IX+d)
                case 0xBE:          // CP (IX+d)
                            len = 3;
                            break;
                case 0x21:          // LD IX,nn'
                case 0x22:          // LD (nn'),IX
                case 0x2A:          // LD IX,(nn')
                case 0x36:          // LD (IX+d),n
                case 0xCB:          // Rotation (IX+d)
                            len = 4;
                            break;
                }
                break;
    case 0xED:  len = 2;
                switch(Opcodes[p+1]) {// 2.Teil des Opcodes
		case 0x4D:
			*type = 'r';
			break;
                case 0x43:          // LD (nn'),BC
                case 0x4B:          // LD BC,(nn')
                case 0x53:          // LD (nn'),DE
                case 0x5B:          // LD DE,(nn')
                case 0x73:          // LD (nn'),SP
                case 0x7B:          // LD SP,(nn')
                            len = 4;
                            break;
                }
                break;
    case 0xFD:  len = 2;
                switch(Opcodes[p+1]) {// 2.Teil des Opcodes
                case 0x34:          // INC (IY+d)
                case 0x35:          // DEC (IY+d)
                case 0x46:          // LD B,(IY+d)
                case 0x4E:          // LD C,(IY+d)
                case 0x56:          // LD D,(IY+d)
                case 0x5E:          // LD E,(IY+d)
                case 0x66:          // LD H,(IY+d)
                case 0x6E:          // LD L,(IY+d)
                case 0x70:          // LD (IY+d),B
                case 0x71:          // LD (IY+d),C
                case 0x72:          // LD (IY+d),D
                case 0x73:          // LD (IY+d),E
                case 0x74:          // LD (IY+d),H
                case 0x75:          // LD (IY+d),L
                case 0x77:          // LD (IY+d),A
                case 0x7E:          // LD A,(IY+d)
                case 0x86:          // ADD A,(IY+d)
                case 0x8E:          // ADC A,(IY+d)
                case 0x96:          // SUB A,(IY+d)
                case 0x9E:          // SBC A,(IY+d)
                case 0xA6:          // AND (IY+d)
                case 0xAE:          // XOR (IY+d)
                case 0xB6:          // OR (IY+d)
                case 0xBE:          // CP (IY+d)
                            len = 3;
                            break;
		case 0xe9:
			*type = 'b';
			break;
                case 0x21:          // LD IY,nn'
                case 0x22:          // LD (nn'),IY
                case 0x2A:          // LD IY,(nn')
                case 0x36:          // LD (IY+d),n
                case 0xCB:          // Rotation,Bitop (IY+d)
                            len = 4;
                            break;
                }
                break;
    }
    return(len);
}
 
void        ParseOpcodes(UBYTE *Opcodes, ULONG adr)
{
WORD    i,len;
ULONG   next;
Boolean label = true;
 
    do {
        if(label)                       // ein Label setzen?
            OpcodesFlags[adr] |= 0x10;  // Label setzen
        if((OpcodesFlags[adr] & 0x0F) == Opcode) break; // Schleife erkannt!
        if((OpcodesFlags[adr] & 0x0F) == Operand) {
            //DebugStr("\pIllegaler Sprung?!?");
            return;
        }
        len = z80_OpcodeLen(Opcodes, adr, NULL);           // Länge vom Opcode ermitteln
        for(i=0;i<len;i++)
            OpcodesFlags[adr+i] = Operand;  // Opcode eintragen
        OpcodesFlags[adr] = Opcode;     // Start des Opcodes markieren
        if(label) {                     // ein Label setzen?
            OpcodesFlags[adr] |= 0x10;  // Label setzen
            label = false;              // Label-Flag zurücksetzen
        }
 
        next = adr + len;               // Ptr auf den Folgeopcode
        switch(Opcodes[adr]) {          // Opcode holen
        case 0xCA:      // JP c,????
        case 0xC2:
        case 0xDA:
        case 0xD2:
        case 0xEA:
        case 0xE2:
        case 0xFA:
        case 0xF2:
                ParseOpcodes(Opcodes, (Opcodes[adr+2]<<8) + Opcodes[adr+1]);
                break;
        case 0x28:      // JR c,??
        case 0x20:
        case 0x38:
        case 0x30:
                ParseOpcodes(Opcodes, adr + 2 + (BYTE)Opcodes[adr+1]);
                break;
        case 0xCC:      // CALL c,????
        case 0xC4:
        case 0xDC:
        case 0xD4:
        case 0xEC:
        case 0xE4:
        case 0xFC:
        case 0xF4:
                ParseOpcodes(Opcodes, (Opcodes[adr+2]<<8) + Opcodes[adr+1]);
                break;
        case 0xC8:      // RET c
        case 0xC0:
        case 0xD8:
        case 0xD0:
        case 0xE8:
        case 0xE0:
        case 0xF8:
        case 0xF0:
                break;
        case 0xC7:      // RST 0
        case 0xCF:      // RST 8
        case 0xD7:      // RST 10
        case 0xDF:      // RST 18
        case 0xE7:      // RST 20
        case 0xEF:      // RST 28
        case 0xF7:      // RST 30
        case 0xFF:      // RST 38
                ParseOpcodes(Opcodes, Opcodes[adr] & 0x38);
                break;
        case 0x10:      // DJNZ ??
                ParseOpcodes(Opcodes, adr + 2 + (BYTE)Opcodes[adr+1]);
                break;
        case 0xC3:      // JP ????
                next = (Opcodes[adr+2]<<8) + Opcodes[adr+1];
                label = true;
                break;
        case 0x18:      // JR ??
                next = adr + 2 + (BYTE)Opcodes[adr+1];
                label = true;
                break;
        case 0xCD:      // CALL ????
                ParseOpcodes(Opcodes, (Opcodes[adr+2]<<8) + Opcodes[adr+1]);
                break;
        case 0xC9:      // RET
                return;
        case 0xE9:
#if DEBUGGER
                DebugStr("\pJP (HL) gefunden"); // JP (HL)
#endif
                label = true;
                break;
        case 0xDD:
#if DEBUGGER
                if(Opcodes[adr+1] == 0xE9) {    // JP (IX)
                    DebugStr("\pJP (IX) gefunden");
                }
#endif
                label = true;
                break;
        case 0xFD:
#if DEBUGGER
                if(Opcodes[adr+1] == 0xE9) {    // JP (IY)
                    DebugStr("\pJP (IY) gefunden");
                }
#endif
                label = true;
                break;
        case 0xED:
                if(Opcodes[adr+1] == 0x4D) {    // RTI
                    return;
                } else if(Opcodes[adr+1] == 0x45) { // RETN
                    return;
                }
                break;
        }
        adr = next;
    } while(1);
}
 
// Disassemblieren
VOID        z80_Disassemble(UBYTE *Opcodes, UWORD adr,STR s)
{
UBYTE           a = Opcodes[adr];
UBYTE           d = (a >> 3) & 7;
UBYTE           e = a & 7;
static STR      reg[8] = {"b","c","d","e","h","l","(hl)","a"};
static STR      dreg[4] = {"bc","de","hl","sp"};
static STR      cond[8] = {"nz","z","nc","c","po","pe","p","m"};
static STR      arith[8] = {"add a, ","adc a, ","sub ","sbc a, ","and ","xor ","or ","cp "};
CHAR            stemp[80];      // temp.String für sprintf()
CHAR            ireg[3];        // temp.Indexregister
 
    switch(a & 0xC0) {
    case 0x00:
        switch(e) {
        case 0x00:
            switch(d) {
            case 0x00:
                strcpy(s, "nop");
                break;
            case 0x01:
                strcpy(s,"ex af, af");
                break;
            case 0x02:
                strcpy(s,"djnz ");
                sprintf(stemp,"0x%4.4x",adr+2+(BYTE)Opcodes[adr+1]);strcat(s,stemp);
                break;
            case 0x03:
                strcpy(s,"jr ");
                sprintf(stemp,"0x%4.4x",adr+2+(BYTE)Opcodes[adr+1]);strcat(s,stemp);
                break;
            default:
                strcpy(s,"jr ");
                strcat(s,cond[d & 3]);
                strcat(s,", ");
                sprintf(stemp,"0x%4.4x",adr+2+(BYTE)Opcodes[adr+1]);strcat(s,stemp);
                break;
            }
            break;
        case 0x01:
            if(a & 0x08) {
                strcpy(s,"add hl,");
                strcat(s,dreg[d >> 1]);
            } else {
                strcpy(s,"ld ");
                strcat(s,dreg[d >> 1]);
                strcat(s,", ");
                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
            }
            break;
        case 0x02:
            switch(d) {
            case 0x00:
                strcpy(s,"ld (bc),a"); //LD\t\t(BC),A");
                break;
            case 0x01:
                strcpy(s,"ld a, (bc)"); //LD\tA,(BC)");
                break;
            case 0x02:
                strcpy(s,"ld (de), a"); //LD\t\t(DE),A");
                break;
            case 0x03:
                strcpy(s,"ld a, (de)"); //LD\t\tA,(DE)");
                break;
            case 0x04:
                strcpy(s,"ld (");
                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                strcat(s,"), hl");
                break;
            case 0x05:
                strcpy(s,"ld hl, (");
                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                strcat(s,")");
                break;
            case 0x06:
                strcpy(s,"ld (");
                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                strcat(s,"), a");
                break;
            case 0x07:
                strcpy(s,"ld a, (");
                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                strcat(s,")");
                break;
            }
            break;
        case 0x03:
            if(a & 0x08)
                strcpy(s,"dec "); //\t\t");
            else
                strcpy(s,"inc ");
            strcat(s,dreg[d >> 1]);
            break;
        case 0x04:
            strcpy(s,"inc ");
            strcat(s,reg[d]);
            break;
        case 0x05:
            strcpy(s,"dec ");
            strcat(s,reg[d]);
            break;
        case 0x06:              // LD   d,n
            strcpy(s,"ld ");
            strcat(s,reg[d]);
            strcat(s,", ");
            sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
            break;
        case 0x07:
            {
            static STR str[8] = {"rlca", "rrca", "rla", "rra", "daa", "cpl", "scf", "ccf" };
            strcpy(s,str[d]);
            }
            break;
        }
        break;
    case 0x40:                          // LD   d,s
        if(d == e) {
            strcpy(s,"halt");
        } else {
            strcpy(s,"ld ");
            strcat(s,reg[d]);
            strcat(s,", ");
            strcat(s,reg[e]);
        }
        break;
    case 0x80:
        strcpy(s,arith[d]);
        strcat(s,reg[e]);
        break;
    case 0xC0:
        switch(e) {
        case 0x00:
            strcpy(s,"ret ");
            strcat(s, cond[d]);
            break;
        case 0x01:
            if(d & 1) {
                switch(d >> 1) {
                case 0x00:
                    strcpy(s,"ret");
                    break;
                case 0x01:
                    strcpy(s,"exx");
                    break;
                case 0x02:
                    strcpy(s,"jp (hl)");
                    break;
                case 0x03:
                    strcpy(s,"ld sp, hl");
                    break;
                }
            } else {
                strcpy(s,"pop ");
                if((d >> 1)==3)
                    strcat(s,"af");
                else
                    strcat(s,dreg[d >> 1]);
            }
            break;
        case 0x02:
            strcpy(s,"jp ");
            strcat(s,cond[d]);
            strcat(s,", ");
            sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
            break;
        case 0x03:
            switch(d) {
            case 0x00:
                strcpy(s,"jp ");
                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                break;
            case 0x01:                  // 0xCB
                a = Opcodes[++adr];     // Erweiterungsopcode holen
                d = (a >> 3) & 7;
                e = a & 7;
                stemp[1] = 0;           // temp.String = 1 Zeichen
                switch(a & 0xC0) {
                case 0x00:
                    {
                    static STR str[8] = {"rlc", "rrc", "rl", "rr", "sla", "sra", "???", "srl"};
                    strcpy(s,str[d]);
                    }
                    strcat(s," ");
                    strcat(s,reg[e]);
                    break;
                case 0x40:
                    strcpy(s,"bit ");
                    stemp[0] = d+'0';strcat(s,stemp);
                    strcat(s,", ");
                    strcat(s,reg[e]);
                    break;
                case 0x80:
                    strcpy(s,"res ");
                    stemp[0] = d+'0';strcat(s,stemp);
                    strcat(s,", ");
                    strcat(s,reg[e]);
                    break;
                case 0xC0:
                    strcpy(s,"set ");
                    stemp[0] = d+'0';strcat(s,stemp);
                    strcat(s,", ");
                    strcat(s,reg[e]);
                    break;
                }
                break;
            case 0x02:
                strcpy(s,"out (");
                sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                strcat(s,"), a");
                break;
            case 0x03:
                strcpy(s,"in a, (");
                sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                strcat(s,")");
                break;
            case 0x04:
                strcpy(s,"ex (sp), hl");
                break;
            case 0x05:
                strcpy(s,"ex de, hl");
                break;
            case 0x06:
                strcpy(s,"di");
                break;
            case 0x07:
                strcpy(s,"ei");
                break;
            }
            break;
        case 0x04:
            strcpy(s,"call ");
            strcat(s,cond[d]);
            strcat(s,", ");
            sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
            break;
        case 0x05:
            if(d & 1) {
                switch(d >> 1) {
                case 0x00:
                    strcpy(s,"call ");
                    sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                    break;
                case 0x02:              // 0xED
                    a = Opcodes[++adr]; // Erweiterungsopcode holen
                    d = (a >> 3) & 7;
                    e = a & 7;
                    switch(a & 0xC0) {
                    case 0x40:
                        switch(e) {
                        case 0x00:
                            strcpy(s,"in ");
                            strcat(s,reg[d]);
                            strcat(s,", (c)");
                            break;
                        case 0x01:
                            strcpy(s,"out (c),");
                            strcat(s,reg[d]);
                            break;
                        case 0x02:
                            if(d & 1)
                                strcpy(s,"adc");
                            else
                                strcpy(s,"sbc");
                            strcat(s," hl, ");
                            strcat(s,dreg[d >> 1]);
                            break;
                        case 0x03:
                            if(d & 1) {
                                strcpy(s,"ld ");
                                strcat(s,dreg[d >> 1]);
                                strcat(s,", (");
                                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                                strcat(s,")");
                            } else {
                                strcpy(s,"LD\t\t(");
                                sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                                strcat(s,"),");
                                strcat(s,dreg[d >> 1]);
                            }
                            break;
                        case 0x04:
                            {
                            static STR str[8] = {"neg","???","???","???","???","???","???","???"};
                            strcpy(s,str[d]);
                            }
                            break;
                        case 0x05:
                            {
                            static STR str[8] = {"retn","reti","???","???","???","???","???","???"};
                            strcpy(s,str[d]);
                            }
                            break;
                        case 0x06:
                            strcpy(s,"im ");
                            stemp[0] = d + '0' - 1; stemp[1] = 0;
                            strcat(s,stemp);
                            break;
                        case 0x07:
                            {
                            static STR str[8] = {"ld i, a","???","ld a,i","???","rrd","rld","???","???"};
                            strcpy(s,str[d]);
                            }
                            break;
                        }
                        break;
                    case 0x80:
                        {
                        static STR str[32] = {"ldi","cpi","ini","outi","???","???","???","???",
                                              "ldd","cpd","ind","outd","???","???","???","???",
                                              "ldir","cpir","inir","otir","???","???","???","???",
                                              "lddr","cpdr","indr","otdr","???","???","???","???"};
                        strcpy(s,str[a & 0x1F]);
                        }
                        break;
                    }
                    break;
                default:                // 0x01 (0xDD) = IX, 0x03 (0xFD) = IY
                    strcpy(ireg,(a & 0x20)?"iy":"ix");
                    a = Opcodes[++adr]; // Erweiterungsopcode holen
                    switch(a) {
                    case 0x09:
                        strcpy(s,"add ");
                        strcat(s,ireg);
                        strcat(s,", bc");
                        break;
                    case 0x19:
                        strcpy(s,"add ");
                        strcat(s,ireg);
                        strcat(s,", de");
                        break;
                    case 0x21:
                        strcpy(s,"ld ");
                        strcat(s,ireg);
                        strcat(s,", ");
                        sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                        break;
                    case 0x22:
                        strcpy(s,"ld (");
                        sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                        strcat(s,"), ");
                        strcat(s,ireg);
                        break;
                    case 0x23:
                        strcpy(s,"inc ");
                        strcat(s,ireg);
                        break;
                    case 0x29:
                        strcpy(s,"add ");
                        strcat(s,ireg);
                        strcat(s,", ");
                        strcat(s,ireg);
                        break;
                    case 0x2A:
                        strcpy(s,"ld ");
                        strcat(s,ireg);
                        strcat(s,", (");
                        sprintf(stemp,"0x%4.4x",Opcodes[adr+1]+(Opcodes[adr+2]<<8));strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x2B:
                        strcpy(s,"dec ");
                        strcat(s,ireg);
                        break;
                    case 0x34:
                        strcpy(s,"inc (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x35:
                        strcpy(s,"dec (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x36:
                        strcpy(s,"ld (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,"),");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+2]);strcat(s,stemp);
                        break;
                    case 0x39:
                        strcpy(s,"add ");
                        strcat(s,ireg);
                        strcat(s,", sp");
                        break;
                    case 0x46:
                    case 0x4E:
                    case 0x56:
                    case 0x5E:
                    case 0x66:
                    case 0x6E:
                        strcpy(s,"ld ");
                        strcat(s,reg[(a>>3)&7]);
                        strcat(s,",(");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x70:
                    case 0x71:
                    case 0x72:
                    case 0x73:
                    case 0x74:
                    case 0x75:
                    case 0x77:
                        strcpy(s,"ld (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,"),");
                        strcat(s,reg[a & 7]);
                        break;
                    case 0x7E:
                        strcpy(s,"ld a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x86:
                        strcpy(s,"add a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x8E:
                        strcpy(s,"adc a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x96:
                        strcpy(s,"sub (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0x9E:
                        strcpy(s,"sbc a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0xA6:
                        strcpy(s,"and a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0xAE:
                        strcpy(s,"xor a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0xB6:
                        strcpy(s,"or a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0xBE:
                        strcpy(s,"cp a, (");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    case 0xE1:
                        strcpy(s,"pop ");
                        strcat(s,ireg);
                        break;
                    case 0xE3:
                        strcpy(s,"ex (sp), ");
                        strcat(s,ireg);
                        break;
                    case 0xE5:
                        strcpy(s,"push ");
                        strcat(s,ireg);
                        break;
                    case 0xE9:
                        strcpy(s,"jp (");
                        strcat(s, ireg);
                        strcat(s,")");
                        break;
                    case 0xF9:
                        strcpy(s,"ld sp, ");
                        strcat(s,ireg);
                        break;
                    case 0xCB:
                        a = Opcodes[adr+2]; // weiteren Unteropcode
                        d = (a >> 3) & 7;
                        stemp[1] = 0;
                        switch(a & 0xC0) {
                        case 0x00:
                            {
                            static STR str[8] = {"RLC","RRC","RL","RR","SLA","SRA","???","SRL"};
                            strcpy(s,str[d]);
                            }
                            strcat(s," ");
                            break;
                        case 0x40:
                            strcpy(s,"bit ");
                            stemp[0] = d + '0';
                            strcat(s,stemp);
                            strcat(s,", ");
                            break;
                        case 0x80:
                            strcpy(s,"res");
                            stemp[0] = d + '0';
                            strcat(s,stemp);
                            strcat(s,", ");
                            break;
                        case 0xC0:
                            strcpy(s,"set ");
                            stemp[0] = d + '0';
                            strcat(s,stemp);
                            strcat(s,", ");
                            break;
                        }
                        strcat(s,"(");
                        strcat(s,ireg);
                        strcat(s,"+");
                        sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
                        strcat(s,")");
                        break;
                    }
                    break;
                }
            } else {
                strcpy(s,"push ");
                if((d >> 1)==3)
                    strcat(s,"af");
                else
                    strcat(s,dreg[d >> 1]);
            }
            break;
        case 0x06:
            strcpy(s,arith[d]);
            sprintf(stemp,"0x%2.2x",Opcodes[adr+1]);strcat(s,stemp);
            break;
        case 0x07:
            strcpy(s,"rst ");
            sprintf(stemp,"0x%2.2x",a & 0x38);strcat(s,stemp);
            break;
        }
        break;
    }
}
 
#if C_MAIN
// Einlesen, Parsen, Disassemblieren und Ausgeben
int main(void)
{
WORD    i;
FILE    *f;
UWORD   adr = 0;
CHAR    s[80];          // Ausgabestring
UBYTE       Opcodes[CODESIZE];

	memcpy(Opcodes, "\x90\x32\x84\x11\x10", 5);
	for(i=0;i<5;i++) {
		int len = z80_OpcodeLen (Opcodes, i, NULL);
		z80_Disassemble (Opcodes, i, s);
		printf("0x%04x  %d\t%s\n", i, len, s);
	}

return 0;
#if 0
 
    //f = fopen("EPROM","rb");
f = stdin;
    if(!f) return;
    fread(Opcodes,CODESIZE,1,f);    // EPROM einlesen
    fclose(f);
 
    for(i=0;i<CODESIZE;i++)         // alles Daten…
        OpcodesFlags[i] = Data;
    for(i=0;i<0x40;i+=0x08)
        if((OpcodesFlags[i] & 0x0F) == Data)
            ParseOpcodes(Opcodes, i);        // RST-Vektoren parsen (wenn nötig)
    if((OpcodesFlags[i] & 0x0F) == Data)
        ParseOpcodes(Opcodes, 0x66);         // NMI-Vektor auch parsen (wenn nötig)
 
#if FUTURA_189
    ParseOpcodes(Opcodes, 0xA41);
    ParseOpcodes(Opcodes, 0xDB6);        // Meßwerte darstellen
    ParseOpcodes(Opcodes, 0xF5D);
    ParseOpcodes(Opcodes, 0xE83);
 
    ParseOpcodes(Opcodes, 0x0978);
    ParseOpcodes(Opcodes, 0x0933);
    ParseOpcodes(Opcodes, 0x11D3);
    ParseOpcodes(Opcodes, 0x1292);
    ParseOpcodes(Opcodes, 0x0AF8);
    ParseOpcodes(Opcodes, 0x098F);
    ParseOpcodes(Opcodes, 0x0B99);
    ParseOpcodes(Opcodes, 0x0BB3);
    ParseOpcodes(Opcodes, 0x0B4A);       // Tastenfeld
    ParseOpcodes(Opcodes, 0x0B12);
    ParseOpcodes(Opcodes, 0x08FF);
    ParseOpcodes(Opcodes, 0x08F0);
    ParseOpcodes(Opcodes, 0x0BDA);
    ParseOpcodes(Opcodes, 0x0BCD);
    ParseOpcodes(Opcodes, 0x0A7E);
    ParseOpcodes(Opcodes, 0x0C2D);
    ParseOpcodes(Opcodes, 0x0AA6);
    ParseOpcodes(Opcodes, 0x0848);
 
    ParseOpcodes(Opcodes, 0x1660);
    ParseOpcodes(Opcodes, 0x166E);
    ParseOpcodes(Opcodes, 0x167C);       // Spezielle Tastenkombinationen
    ParseOpcodes(Opcodes, 0x168A);
    ParseOpcodes(Opcodes, 0x1698);
    ParseOpcodes(Opcodes, 0x16A6);
    ParseOpcodes(Opcodes, 0x16CF);
#endif
 
    f = stdout;
    //f = fopen("OUTPUT","w");
    if(!f) return;
    while(adr < CODESIZE) {
        WORD    len,i;
 
        if((OpcodesFlags[adr] & 0x0F) == Data) {
            fprintf(f,"L0x%4.4x:\tDEFB",(UWORD)adr);
            for(i=0;i<16;i++) {
                if((OpcodesFlags[adr+i] & 0x0F) != Data) break;
                fprintf(f,"%c0x%2.2x",(i)?',':' ',Opcodes[adr+i]);
            }
            fprintf(f,"\n");
            adr += i;
        } else {
            len = z80_OpcodeLen(Opcodes, adr, NULL);           // Länge vom Opcode ermitteln
#if 1
            if(OpcodesFlags[adr] & 0x10)
                fprintf(f,"L0x%4.4x:\t",adr);
            else
                fprintf(f,"\t\t");
#else
            fprintf(f,"0x%4.4x: ",(UWORD)adr);
            for(i=0;i<len;i++)
                fprintf(f,"%2.2X ",Opcodes[adr+i]);
            for(i=4;i>len;i--)
                fprintf(f,"   ");
            fprintf(f," ");
#endif
            z80_Disassemble(Opcodes, adr,s);
            fprintf(f,"%s\n",s);
            adr += len;
        }
    }
    fclose(f);
#endif
return 0;
}
#endif
