#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#define MAX_JSON_SIZE 65536
#define MAX_TEXT_SIZE 8192

typedef struct {
    char txid[256];
    int size;
    int vsize;
    int weight;
    char scriptSigAsm[MAX_TEXT_SIZE];
    char scriptPubKeyAsm[MAX_TEXT_SIZE];
    char scriptPubKeyType[256];
    char txinwitness[MAX_TEXT_SIZE];
} TxInfo;

/* ------------------------------ Utility Functions ------------------------------ */

static int read_file(const char *filename, char *buffer, size_t buffer_size) {
    FILE *fp;
    size_t nread;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open file %s\n", filename);
        return -1;
    }

    nread = fread(buffer, 1, buffer_size - 1, fp);
    buffer[nread] = '\0';
    fclose(fp);

    return 0;
}

static void safe_copy(char *dest, size_t dest_size, const char *src) {
    if (src == NULL) {
        snprintf(dest, dest_size, "N/A");
        return;
    }
    snprintf(dest, dest_size, "%s", src);
}

static void extract_scriptSig(cJSON *root, TxInfo *info) {
    cJSON *vin = cJSON_GetObjectItemCaseSensitive(root, "vin");
    cJSON *vin0;
    cJSON *scriptSig;
    cJSON *asm_item;

    snprintf(info->scriptSigAsm, sizeof(info->scriptSigAsm), "N/A");

    if (!cJSON_IsArray(vin) || cJSON_GetArraySize(vin) == 0) {
        return;
    }

    vin0 = cJSON_GetArrayItem(vin, 0);
    if (vin0 == NULL) {
        return;
    }

    scriptSig = cJSON_GetObjectItemCaseSensitive(vin0, "scriptSig");
    if (!cJSON_IsObject(scriptSig)) {
        return;
    }

    asm_item = cJSON_GetObjectItemCaseSensitive(scriptSig, "asm");
    if (cJSON_IsString(asm_item) && asm_item->valuestring != NULL) {
        safe_copy(info->scriptSigAsm, sizeof(info->scriptSigAsm), asm_item->valuestring);
    }
}

static void extract_scriptPubKey(cJSON *root, TxInfo *info) {
    cJSON *vout = cJSON_GetObjectItemCaseSensitive(root, "vout");
    cJSON *vout0;
    cJSON *scriptPubKey;
    cJSON *asm_item;
    cJSON *type_item;

    snprintf(info->scriptPubKeyAsm, sizeof(info->scriptPubKeyAsm), "N/A");
    snprintf(info->scriptPubKeyType, sizeof(info->scriptPubKeyType), "N/A");

    if (!cJSON_IsArray(vout) || cJSON_GetArraySize(vout) == 0) {
        return;
    }

    vout0 = cJSON_GetArrayItem(vout, 0);
    if (vout0 == NULL) {
        return;
    }

    scriptPubKey = cJSON_GetObjectItemCaseSensitive(vout0, "scriptPubKey");
    if (!cJSON_IsObject(scriptPubKey)) {
        return;
    }

    asm_item = cJSON_GetObjectItemCaseSensitive(scriptPubKey, "asm");
    type_item = cJSON_GetObjectItemCaseSensitive(scriptPubKey, "type");

    if (cJSON_IsString(asm_item) && asm_item->valuestring != NULL) {
        safe_copy(info->scriptPubKeyAsm, sizeof(info->scriptPubKeyAsm), asm_item->valuestring);
    }

    if (cJSON_IsString(type_item) && type_item->valuestring != NULL) {
        safe_copy(info->scriptPubKeyType, sizeof(info->scriptPubKeyType), type_item->valuestring);
    }
}

static void extract_txinwitness(cJSON *root, TxInfo *info) {
    cJSON *vin = cJSON_GetObjectItemCaseSensitive(root, "vin");
    cJSON *vin0;
    cJSON *txinwitness;
    int i, count;

    snprintf(info->txinwitness, sizeof(info->txinwitness), "N/A");

    if (!cJSON_IsArray(vin) || cJSON_GetArraySize(vin) == 0) {
        return;
    }

    vin0 = cJSON_GetArrayItem(vin, 0);
    if (vin0 == NULL) {
        return;
    }

    txinwitness = cJSON_GetObjectItemCaseSensitive(vin0, "txinwitness");
    if (!cJSON_IsArray(txinwitness)) {
        return;
    }

    info->txinwitness[0] = '\0';
    count = cJSON_GetArraySize(txinwitness);

    for (i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(txinwitness, i);
        if (cJSON_IsString(item) && item->valuestring != NULL) {
            if (strlen(info->txinwitness) + strlen(item->valuestring) + 4 < sizeof(info->txinwitness)) {
                strcat(info->txinwitness, item->valuestring);
                if (i != count - 1) {
                    strcat(info->txinwitness, " | ");
                }
            }
        }
    }

    if (info->txinwitness[0] == '\0') {
        snprintf(info->txinwitness, sizeof(info->txinwitness), "N/A");
    }
}

static int parse_transaction_json(const char *json_text, TxInfo *info) {
    cJSON *root;
    cJSON *txid_item;
    cJSON *size_item;
    cJSON *vsize_item;
    cJSON *weight_item;

    memset(info, 0, sizeof(TxInfo));
    snprintf(info->txid, sizeof(info->txid), "N/A");
    snprintf(info->scriptSigAsm, sizeof(info->scriptSigAsm), "N/A");
    snprintf(info->scriptPubKeyAsm, sizeof(info->scriptPubKeyAsm), "N/A");
    snprintf(info->scriptPubKeyType, sizeof(info->scriptPubKeyType), "N/A");
    snprintf(info->txinwitness, sizeof(info->txinwitness), "N/A");

    root = cJSON_Parse(json_text);
    if (root == NULL) {
        printf("Error: invalid JSON data\n");
        return -1;
    }

    txid_item = cJSON_GetObjectItemCaseSensitive(root, "txid");
    size_item = cJSON_GetObjectItemCaseSensitive(root, "size");
    vsize_item = cJSON_GetObjectItemCaseSensitive(root, "vsize");
    weight_item = cJSON_GetObjectItemCaseSensitive(root, "weight");

    if (cJSON_IsString(txid_item) && txid_item->valuestring != NULL) {
        safe_copy(info->txid, sizeof(info->txid), txid_item->valuestring);
    }

    info->size = cJSON_IsNumber(size_item) ? size_item->valueint : -1;
    info->vsize = cJSON_IsNumber(vsize_item) ? vsize_item->valueint : -1;
    info->weight = cJSON_IsNumber(weight_item) ? weight_item->valueint : -1;

    extract_scriptSig(root, info);
    extract_scriptPubKey(root, info);
    extract_txinwitness(root, info);

    cJSON_Delete(root);
    return 0;
}

static void print_transaction_info(const char *title, const TxInfo *info) {
    printf("\n==================================================\n");
    printf("%s\n", title);
    printf("==================================================\n");
    printf("txid              : %s\n", info->txid);
    printf("size              : %d\n", info->size);
    printf("vsize             : %d\n", info->vsize);
    printf("weight            : %d\n", info->weight);
    printf("scriptSig.asm     : %s\n", info->scriptSigAsm);
    printf("scriptPubKey.asm  : %s\n", info->scriptPubKeyAsm);
    printf("scriptPubKey.type : %s\n", info->scriptPubKeyType);
    printf("txinwitness       : %s\n", info->txinwitness);
}

static void print_comparison_table(const TxInfo *legacy_ab,
                                   const TxInfo *legacy_bc,
                                   const TxInfo *segwit_ab,
                                   const TxInfo *segwit_bc) {
    printf("\n==================================================\n");
    printf("LEGACY VS SEGWIT COMPARISON\n");
    printf("==================================================\n");
    printf("%-18s %-8s %-8s %-8s %-15s\n", "Transaction", "Size", "Vsize", "Weight", "Type");
    printf("-----------------------------------------------------------------\n");
    printf("%-18s %-8d %-8d %-8d %-15s\n", "Legacy A->B",
           legacy_ab->size, legacy_ab->vsize, legacy_ab->weight, legacy_ab->scriptPubKeyType);
    printf("%-18s %-8d %-8d %-8d %-15s\n", "Legacy B->C",
           legacy_bc->size, legacy_bc->vsize, legacy_bc->weight, legacy_bc->scriptPubKeyType);
    printf("%-18s %-8d %-8d %-8d %-15s\n", "SegWit A'->B'",
           segwit_ab->size, segwit_ab->vsize, segwit_ab->weight, segwit_ab->scriptPubKeyType);
    printf("%-18s %-8d %-8d %-8d %-15s\n", "SegWit B'->C'",
           segwit_bc->size, segwit_bc->vsize, segwit_bc->weight, segwit_bc->scriptPubKeyType);
}

static void print_summary_analysis(const TxInfo *legacy_bc, const TxInfo *segwit_bc) {
    double vsize_reduction = 0.0;
    double weight_reduction = 0.0;

    if (legacy_bc->vsize > 0) {
        vsize_reduction = ((double)(legacy_bc->vsize - segwit_bc->vsize) / legacy_bc->vsize) * 100.0;
    }

    if (legacy_bc->weight > 0) {
        weight_reduction = ((double)(legacy_bc->weight - segwit_bc->weight) / legacy_bc->weight) * 100.0;
    }

    printf("\n==================================================\n");
    printf("ANALYSIS SUMMARY\n");
    printf("==================================================\n");
    printf("Legacy transactions store unlocking data directly in scriptSig.\n");
    printf("SegWit transactions move signature/public key data into txinwitness.\n");
    printf("This reduces fee-relevant size even if raw size can look larger.\n\n");

    printf("Legacy B->C  : size=%d, vsize=%d, weight=%d\n",
           legacy_bc->size, legacy_bc->vsize, legacy_bc->weight);
    printf("SegWit B'->C': size=%d, vsize=%d, weight=%d\n",
           segwit_bc->size, segwit_bc->vsize, segwit_bc->weight);

    printf("\nVsize reduction  : %.2f%%\n", vsize_reduction);
    printf("Weight reduction : %.2f%%\n", weight_reduction);

    printf("\nLegacy scriptPubKey type : %s\n", legacy_bc->scriptPubKeyType);
    printf("SegWit scriptPubKey type : %s\n", segwit_bc->scriptPubKeyType);
    printf("Legacy scriptSig         : %s\n", legacy_bc->scriptSigAsm);
    printf("SegWit scriptSig         : %s\n", segwit_bc->scriptSigAsm);
    printf("SegWit txinwitness       : %s\n", segwit_bc->txinwitness);
}

int main(void) {
    char json_text[MAX_JSON_SIZE];

    TxInfo legacy_ab;
    TxInfo legacy_bc;
    TxInfo segwit_ab;
    TxInfo segwit_bc;

    /* ========================== START OF LEGACY P2PKH SECTION ========================== */

    if (read_file("legacy_A_B.json", json_text, sizeof(json_text)) != 0) {
        return 1;
    }
    if (parse_transaction_json(json_text, &legacy_ab) != 0) {
        return 1;
    }
    print_transaction_info("LEGACY P2PKH TRANSACTION A -> B", &legacy_ab);

    if (read_file("legacy_B_C.json", json_text, sizeof(json_text)) != 0) {
        return 1;
    }
    if (parse_transaction_json(json_text, &legacy_bc) != 0) {
        return 1;
    }
    print_transaction_info("LEGACY P2PKH TRANSACTION B -> C", &legacy_bc);

    /* =========================== END OF LEGACY P2PKH SECTION =========================== */


    /* ===================== START OF SEGWIT P2SH-P2WPKH SECTION ===================== */

    if (read_file("segwit_A_B.json", json_text, sizeof(json_text)) != 0) {
        return 1;
    }
    if (parse_transaction_json(json_text, &segwit_ab) != 0) {
        return 1;
    }
    print_transaction_info("SEGWIT P2SH-P2WPKH TRANSACTION A' -> B'", &segwit_ab);

    if (read_file("segwit_B_C.json", json_text, sizeof(json_text)) != 0) {
        return 1;
    }
    if (parse_transaction_json(json_text, &segwit_bc) != 0) {
        return 1;
    }
    print_transaction_info("SEGWIT P2SH-P2WPKH TRANSACTION B' -> C'", &segwit_bc);

    /* ====================== END OF SEGWIT P2SH-P2WPKH SECTION ====================== */


    /* ============================== COMPARISON SECTION ============================== */

    print_comparison_table(&legacy_ab, &legacy_bc, &segwit_ab, &segwit_bc);
    print_summary_analysis(&legacy_bc, &segwit_bc);

    /* ============================ END OF COMPARISON SECTION ============================ */

    return 0;
}