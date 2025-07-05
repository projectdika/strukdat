#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TUGAS 100
#define MAX_STACK 50
#define MAX_ANAK 10

#define MERAH "\033[1;31m"
#define KUNING "\033[1;33m"
#define RESET "\033[0m"

typedef struct {
    char deskripsi[100];
    int hari, bulan, tahun;
    int jam, menit;
    int kesulitan; // 1 - 5
    int selesai;
} Tugas;

typedef struct TreeNode {
    Tugas data;
    struct TreeNode* anak[MAX_ANAK];
    int jumlahAnak;
} TreeNode;

Tugas daftarTugas[MAX_TUGAS];
int jumlahTugas = 0;
Tugas stackUndo[MAX_STACK];
int top = -1;
Tugas queueTugas[MAX_TUGAS];
int front = 0, rear = -1, countQueue = 0;
TreeNode* akar = NULL;

int hitungSelisihHari(int d, int m, int y) {
    struct tm deadline = {0}, now = {0};
    time_t t_deadline, t_now;

    deadline.tm_mday = d;
    deadline.tm_mon = m - 1;
    deadline.tm_year = y - 1900;

    time(&t_now);
    struct tm *localNow = localtime(&t_now);
    now = *localNow;
    now.tm_hour = 0;
    now.tm_min = 0;
    now.tm_sec = 0;

    t_deadline = mktime(&deadline);
    t_now = mktime(&now);

    double selisih = difftime(t_deadline, t_now) / (60 * 60 * 24);
    return (int)selisih;
}

int hitungSelisihMenit(int d, int m, int y, int h, int mi) {
    struct tm deadline = {0}, now = {0};
    time_t t_deadline, t_now;

    deadline.tm_mday = d;
    deadline.tm_mon = m - 1;
    deadline.tm_year = y - 1900;
    deadline.tm_hour = h;
    deadline.tm_min = mi;

    time(&t_now);
    struct tm *localNow = localtime(&t_now);
    now = *localNow;

    t_deadline = mktime(&deadline);
    t_now = mktime(&now);

    double selisih = difftime(t_deadline, t_now) / 60;
    return (int)selisih;
}

Tugas popUndo() {
    if (top >= 0) {
        return stackUndo[top--];
    }
    Tugas kosong = {"", 0, 0, 0, 0, 0, 0};
    return kosong;
}

int cariTugas(char keyword[]) {
    for (int i = 0; i < jumlahTugas; i++) {
        if (strstr(daftarTugas[i].deskripsi, keyword)) {
            return i;
        }
    }
    return -1;
}

Tugas dequeue() {
    Tugas t = queueTugas[front];
    front = (front + 1) % MAX_TUGAS;
    countQueue--;
    return t;
}

void tambahTugas() {
    if (jumlahTugas >= MAX_TUGAS) {
        printf("Kapasitas tugas penuh!\n");
        return;
    }

    getchar();
    Tugas t;
    printf("Deskripsi tugas: ");
    fgets(t.deskripsi, sizeof(t.deskripsi), stdin);
    t.deskripsi[strcspn(t.deskripsi, "\n")] = 0;
    printf("Deadline (DD MM YYYY HH MM): ");
    scanf("%d %d %d %d %d", &t.hari, &t.bulan, &t.tahun, &t.jam, &t.menit);
    printf("Kesulitan (1-5): ");
    scanf("%d", &t.kesulitan);
    t.selesai = 0;

    daftarTugas[jumlahTugas++] = t;
    stackUndo[++top] = t;
    rear = (rear + 1) % MAX_TUGAS;
    queueTugas[rear] = t;
    countQueue++;

    printf("Tugas berhasil ditambahkan!\n");
}

TreeNode* buatNode(Tugas data) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->data = data;
    node->jumlahAnak = 0;
    return node;
}

void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
}

TreeNode* cariNode(TreeNode* root, char keyword[]) {
    if (!root) return NULL;
    char deskripsiNode[100], keywordL[100];
    strcpy(deskripsiNode, root->data.deskripsi);
    strcpy(keywordL, keyword);
    toLowerCase(deskripsiNode);
    toLowerCase(keywordL);

    if (strstr(deskripsiNode, keywordL)) return root;
    for (int i = 0; i < root->jumlahAnak; i++) {
        TreeNode* hasil = cariNode(root->anak[i], keyword);
        if (hasil) return hasil;
    }
    return NULL;
}

void tambahSubTugas(TreeNode* parent, Tugas data) {
    if (parent->jumlahAnak < MAX_ANAK) {
        parent->anak[parent->jumlahAnak++] = buatNode(data);
        printf(">> Sub-tugas '%s' berhasil ditambahkan ke '%s'\n", data.deskripsi, parent->data.deskripsi);
    } else {
        printf(">> Gagal menambahkan ke '%s', sudah mencapai batas anak maksimum.\n", parent->data.deskripsi);
    }
}


void tampilTree(TreeNode* root, int level) {
    if (!root) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("- %s (Deadline: %02d/%02d/%04d %02d:%02d)\n",
           root->data.deskripsi,
           root->data.hari, root->data.bulan, root->data.tahun,
           root->data.jam, root->data.menit);
    for (int i = 0; i < root->jumlahAnak; i++) {
        tampilTree(root->anak[i], level + 1);
    }
}

void simpanDaftarTugasCSV() {
    FILE* file = fopen("daftarTugas.csv", "w");
    if (!file) {
        printf("Gagal membuka file untuk menyimpan daftar tugas.\n");
        return;
    }

    fprintf(file, "Deskripsi,Deadline,Jam,Menit,Kesulitan,Selesai\n");
    for (int i = 0; i < jumlahTugas; i++) {
        fprintf(file, "\"%s\",%02d/%02d/%04d,%02d,%02d,%d,%d\n",
            daftarTugas[i].deskripsi,
            daftarTugas[i].hari, daftarTugas[i].bulan, daftarTugas[i].tahun,
            daftarTugas[i].jam, daftarTugas[i].menit,
            daftarTugas[i].kesulitan,
            daftarTugas[i].selesai
        );
    }

    fclose(file);
    printf("Daftar tugas berhasil disimpan ke daftarTugas.csv\n");
}

void simpanTreeCSVRekursif(FILE* file, TreeNode* node, int level) {
    if (!node) return;

    for (int i = 0; i < level; i++) {
        fprintf(file, "-");  // indentasi bertingkat pakai tanda -
    }

    fprintf(file, "\"%s\",%02d/%02d/%04d,%02d,%02d,%d,%d\n",
        node->data.deskripsi,
        node->data.hari, node->data.bulan, node->data.tahun,
        node->data.jam, node->data.menit,
        node->data.kesulitan,
        node->data.selesai
    );

    for (int i = 0; i < node->jumlahAnak; i++) {
        simpanTreeCSVRekursif(file, node->anak[i], level + 1);
    }
}

void simpanTreeCSV() {
    FILE* file = fopen("daftarTugasMeningkat.csv", "w");
    if (!file) {
        printf("Gagal membuka file untuk menyimpan struktur tugas.\n");
        return;
    }

    fprintf(file, "Level,Deskripsi,Deadline,Jam,Menit,Kesulitan,Selesai\n");
    simpanTreeCSVRekursif(file, akar, 0);
    fclose(file);
    printf("Struktur tugas bertingkat berhasil disimpan ke daftarTugasMeningkat.csv\n");
}

void tampilTugas();
void undoTugas();
void tandaiSelesai();
void cariDanTampil();

void menuTree() {
    int pilihan;
    do {
        printf("\n=== Menu Struktur Tugas Bertingkat (Tree) ===\n");
        printf("1. Buat Root Tugas\n");
        printf("2. Tambah Sub-Tugas\n");
        printf("3. Tambah Sub-Sub Tugas\n");
        printf("4. Tampilkan Struktur dengan Deadline\n");
        printf("5. Tandai Selesai Berdasarkan Kata Kunci\n");
        printf("6. Kembali\n");
        printf("Pilih: ");
        scanf("%d", &pilihan);

        if (pilihan == 1) {
            if (akar) {
                printf("Root sudah ada!\n");
                continue;
            }
            getchar();
            Tugas t;
            printf("Deskripsi root: ");
            fgets(t.deskripsi, sizeof(t.deskripsi), stdin);
            t.deskripsi[strcspn(t.deskripsi, "\n")] = 0;
            printf("Deadline root (DD MM YYYY HH MM): ");
            scanf("%d %d %d %d %d", &t.hari, &t.bulan, &t.tahun, &t.jam, &t.menit);
            printf("Kesulitan (1 - 5): ");
            scanf("%d", &t.kesulitan);
            t.selesai = 0;
            akar = buatNode(t);
            printf("Root berhasil dibuat!\n");

        }  else if (pilihan == 2) {
            if (!akar) {
                printf("Buat root terlebih dahulu!\n");
                continue;
            }
            getchar();
            char keyword[100];
            printf("Masukkan kata kunci parent (root) : ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")] = 0;

            TreeNode* parent = cariNode(akar, keyword);
            if (!parent) {
                printf("Parent tidak ditemukan!\n");
                continue;
            }

            Tugas sub;
            printf("Deskripsi sub-tugas: ");
            fgets(sub.deskripsi, sizeof(sub.deskripsi), stdin);
            sub.deskripsi[strcspn(sub.deskripsi, "\n")] = 0;
            printf("Deadline sub-tugas (DD MM YYYY HH MM): ");
            scanf("%d %d %d %d %d", &sub.hari, &sub.bulan, &sub.tahun, &sub.jam, &sub.menit);
            printf("Kesulitan sub-tugas (1 - 5): ");
            scanf("%d", &sub.kesulitan);
            sub.selesai = 0;

            tambahSubTugas(parent, sub);
            printf("Sub-tugas berhasil ditambahkan!\n");
        } else if (pilihan == 3) {
            if (!akar) {
            printf("Buat root terlebih dahulu!\n");
            continue;
            }
            getchar();
            char keyword[100];
            printf("Masukkan kata kunci sub-tugas (parent sub-sub): ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")] = 0;

            TreeNode* parent = cariNode(akar, keyword);
            if (!parent) {
                printf("Parent sub-tugas tidak ditemukan!\n");
                continue;
            }

            Tugas subsub;
            printf("Deskripsi sub-sub-tugas: ");
            fgets(subsub.deskripsi, sizeof(subsub.deskripsi), stdin);
            subsub.deskripsi[strcspn(subsub.deskripsi, "\n")] = 0;
            printf("Deadline sub-sub-tugas (DD MM YYYY HH MM): ");
            scanf("%d %d %d %d %d", &subsub.hari, &subsub.bulan, &subsub.tahun, &subsub.jam, &subsub.menit);
            printf("Kesulitan sub-sub-tugas (1 - 5): ");
            scanf("%d", &subsub.kesulitan);
            subsub.selesai = 0;

            tambahSubTugas(parent, subsub);
            printf("Sub-sub-tugas berhasil ditambahkan!\n");

        } else if (pilihan == 4 ){
            if (!akar) {
                printf("Belum ada struktur tugas.\n");
            } else {
                tampilTreeDenganDeadline(akar, 0);
            }
        }
        
        else if (pilihan == 5) {
            getchar();
            char keyword[100];
            printf("Masukkan kata kunci tugas yang ingin ditandai selesai: ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")] = 0;

            if (tandaiSelesaiNode(akar, keyword)) {
                printf("Tugas berhasil ditandai selesai!\n");
            } else {
                printf("Tugas tidak ditemukan.\n");
            }
        }

    } while (pilihan != 6);
}

void sortTugas() {
    for (int i = 0; i < jumlahTugas - 1; i++) {
        for (int j = 0; j < jumlahTugas - i - 1; j++) {
            Tugas a = daftarTugas[j];
            Tugas b = daftarTugas[j + 1];

            int tglA = a.tahun * 1000000 + a.bulan * 10000 + a.hari * 100 + a.jam;
            int tglB = b.tahun * 1000000 + b.bulan * 10000 + b.hari * 100 + b.jam;

            if (tglA > tglB ||
                (tglA == tglB && a.menit > b.menit) ||
                (tglA == tglB && a.menit == b.menit && a.kesulitan > b.kesulitan)) {
                Tugas temp = daftarTugas[j];
                daftarTugas[j] = daftarTugas[j + 1];
                daftarTugas[j + 1] = temp;
            }
        }
    }
}

void tampilTugas() {
    if (jumlahTugas == 0) {
        printf("\n=== Daftar Tugas (Urut Deadline dan Kesulitan) ===\n");
        printf("------------------------------------------------------\n");
        printf("Maaf, belum ada tugas yang ditambahkan.\n");
        printf("Semoga tidur mu menyenangkan\n");
        return;
    }

    sortTugas();
    printf("\n=== Daftar Tugas (Urut Deadline dan Kesulitan) ===\n");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < jumlahTugas; i++) {
        int selisihHari = hitungSelisihHari(
            daftarTugas[i].hari, 
            daftarTugas[i].bulan, 
            daftarTugas[i].tahun
        );

        int selisihMenit = hitungSelisihMenit(
            daftarTugas[i].hari,
            daftarTugas[i].bulan,
            daftarTugas[i].tahun,
            daftarTugas[i].jam,
            daftarTugas[i].menit
        );

        char statusWaktu[150];
        if (selisihMenit < 0) {
            sprintf(statusWaktu, MERAH "- SUDAH TERLEWAT!!!" RESET);
        } else if (selisihHari == 0) {
            if (selisihMenit <= 60) {
                sprintf(statusWaktu, MERAH "- DEADLINE HARI INI!!!" RESET);
            } else {
                sprintf(statusWaktu, MERAH "- DEADLINE HARI INI!!!" RESET);
            }
        } else if (selisihHari <= 2) {
            sprintf(statusWaktu, KUNING "- lagi %d hari - cepet selesaikan!" RESET, selisihHari);
        } else {
            sprintf(statusWaktu, "- lagi %d hari", selisihHari);
        }

        printf("%d. %s (Deadline: %02d/%02d/%04d %02d:%02d) [Kesulitan: %d] [%s] %s\n",
            i + 1, daftarTugas[i].deskripsi,
            daftarTugas[i].hari, daftarTugas[i].bulan, daftarTugas[i].tahun,
            daftarTugas[i].jam, daftarTugas[i].menit,
            daftarTugas[i].kesulitan,
            daftarTugas[i].selesai ? "Selesai" : "Belum",
            statusWaktu);
    }
}


void undoTugas() {
    if (top < 0) {
        printf("Tidak ada tugas untuk di-undo.\n");
        return;
    }
    Tugas terakhir = popUndo();
    if (jumlahTugas > 0) jumlahTugas--;
    printf("Tugas '%s' berhasil di-undo.\n", terakhir.deskripsi);
}

void tandaiSelesai() {
    tampilTugas();
    int no;
    printf("Masukkan nomor tugas yang ingin ditandai selesai: ");
    scanf("%d", &no);
    if (no < 1 || no > jumlahTugas) {
        printf("Nomor tidak valid!\n");
        return;
    }
    daftarTugas[no - 1].selesai = 1;
    printf("Tugas '%s' berhasil ditandai sebagai selesai!\n", daftarTugas[no - 1].deskripsi);
}

void cariDanTampil() {
    getchar();
    char keyword[100];
    printf("Masukkan kata kunci: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0;

    int idx = cariTugas(keyword);
    if (idx != -1) {
        printf("Tugas ditemukan: %s (Deadline: %02d/%02d/%04d %02d:%02d) [Kesulitan: %d] [%s]\n",
               daftarTugas[idx].deskripsi,
               daftarTugas[idx].hari,
               daftarTugas[idx].bulan,
               daftarTugas[idx].tahun,
               daftarTugas[idx].jam,
               daftarTugas[idx].menit,
               daftarTugas[idx].kesulitan,
               daftarTugas[idx].selesai ? "Selesai" : "Belum");
    }
}

void tampilTreeDenganDeadline(TreeNode* root, int level) {
    if (!root) return;

    int selisihHari = hitungSelisihHari(root->data.hari, root->data.bulan, root->data.tahun);
    int selisihMenit = hitungSelisihMenit(root->data.hari, root->data.bulan, root->data.tahun,
                                          root->data.jam, root->data.menit);

    char statusWaktu[150];
    if (selisihMenit < 0) {
        sprintf(statusWaktu, MERAH "- SUDAH TERLEWAT!!!" RESET);
    } else if (selisihHari == 0) {
        sprintf(statusWaktu, MERAH "- DEADLINE HARI INI!!!" RESET);
    } else if (selisihHari <= 2) {
        sprintf(statusWaktu, KUNING "- lagi %d hari - cepet selesaikan!" RESET, selisihHari);
    } else {
        sprintf(statusWaktu, "- lagi %d hari", selisihHari);
    }

    for (int i = 0; i < level; i++) printf("  ");
    printf("- %s (Deadline: %02d/%02d/%04d %02d:%02d) [Kesulitan: %d] [%s] %s\n",
           root->data.deskripsi,
           root->data.hari, root->data.bulan, root->data.tahun,
           root->data.jam, root->data.menit,
           root->data.kesulitan,
           root->data.selesai ? "Selesai" : "Belum",
           statusWaktu);

    for (int i = 0; i < root->jumlahAnak; i++) {
        tampilTreeDenganDeadline(root->anak[i], level + 1);
    }
}

int tandaiSelesaiNode(TreeNode* root, char keyword[]) {
    if (!root) return 0;
    char deskripsiNode[100], keywordL[100];
    strcpy(deskripsiNode, root->data.deskripsi);
    strcpy(keywordL, keyword);
    toLowerCase(deskripsiNode);
    toLowerCase(keywordL);

    if (strstr(deskripsiNode, keywordL)) {
        root->data.selesai = 1;
        return 1;
    }
    for (int i = 0; i < root->jumlahAnak; i++) {
        if (tandaiSelesaiNode(root->anak[i], keyword)) return 1;
    }
    return 0;
}



int main() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    int pilihan;
    do {
        printf("\n=== Menu Aplikasi SITEPAT ===\n");
        printf("1. Tambah Tugas\n");
        printf("2. Tampil Daftar Tugas\n");
        printf("3. Undo Tambah Tugas (Stack)\n");
        printf("4. Tampil Antrian Tugas (Queue)\n");
        printf("5. Cari Tugas (Searching)\n");
        printf("6. Menu Struktur Tugas Bertingkat (Tree)\n");
        printf("7. Tandai Tugas Selesai\n");
        printf("8. Keluar\n");
        printf("9. Simpan Semua ke CSV\n");
        printf("Pilih: ");
        scanf("%d", &pilihan);

        switch (pilihan) {
            case 1: tambahTugas(); break;
            case 2: tampilTugas(); break;
            case 3: undoTugas(); break;
            case 4:
                if (countQueue > 0) {
                    printf("Tugas dalam antrian: %s\n", dequeue().deskripsi);
                } else {
                    printf("Antrian kosong.\n");
                }
                break;
            case 5: cariDanTampil(); break;
            case 6: menuTree(); break;
            case 7: tandaiSelesai(); break;
            case 8: printf("Terima kasih!\n"); break;
            case 9:
                simpanDaftarTugasCSV();
                simpanTreeCSV();
                break;
            default: printf("Pilihan tidak valid.\n"); break;
        }
    } while (pilihan != 8);
    return 0;
}