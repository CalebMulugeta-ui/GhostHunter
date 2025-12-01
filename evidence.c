#include "defs.h"
#include "helpers.h"

/* 
    Function: evidence_set(EvidenceByte* ev_byte, enum EvidenceType ev_type)
    Purpose:  Sets a specific evidence type bit in an EvidenceByte.
    Params:   
        Input/Output: EvidenceByte* ev_byte - pointer to the EvidenceByte to modify.
        Input: enum EvidenceType ev_type - the specific evidence type bit to set.
    Return: void
*/
void evidence_set(EvidenceByte* ev_byte, enum EvidenceType ev_type) {
    *ev_byte |= ev_type;
}

/* 
    Function: evidence_clear(EvidenceByte* ev_byte, enum EvidenceType ev_type)
    Purpose:  Clears a specific evidence type bit in an EvidenceByte.
    Params:   
        Input/Output: EvidenceByte* ev_byte - pointer to the EvidenceByte to modify.
        Input: enum EvidenceType ev_type - the specific evidence type bit to clear.
    Return: void
*/
void evidence_clear(EvidenceByte* ev_byte, enum EvidenceType ev_type) {
    *ev_byte &= ~ev_type;
}

/* 
    Function: evidence_has(EvidenceByte ev_byte, enum EvidenceType ev_type)
    Purpose:  Checks if a specific evidence type bit is set in an EvidenceByte.
    Params:   
        Input: EvidenceByte ev_byte - the EvidenceByte to check.
        Input: enum EvidenceType ev_type - the specific evidence type bit to check for.
    Return: bool - true if the evidence type is present, false otherwise.
*/
bool evidence_has(EvidenceByte ev_byte, enum EvidenceType ev_type) {
    return (ev_byte & ev_type) != 0;
}

/* 
    Function: evidence_count_bits(EvidenceByte ev_byte)
    Purpose:  Counts the number of set bits (evidence types) in an EvidenceByte.
    Params:   
        Input: EvidenceByte ev_byte - the EvidenceByte to count bits in.
    Return: int - the number of set bits (evidence types present).
*/
int evidence_count_bits(EvidenceByte ev_byte) {
    int cnt = 0;
    for(int b = 0; b < 8; b++) {
        if(ev_byte & (1 << b)) cnt++;
    }
    return cnt;
}

/* 
    Function: evidence_has_three_unique(EvidenceByte ev_mask)
    Purpose:  Determines if an EvidenceByte has at least three unique evidence types set.
    Params:   
        Input: EvidenceByte ev_mask - the EvidenceByte to check.
    Return: bool - true if three or more evidence types are present, false otherwise.
*/
bool evidence_has_three_unique(EvidenceByte ev_mask) {
    return evidence_count_bits(ev_mask) >= 3;
}