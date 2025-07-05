#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TUGAS 100
#define MAX_STACK 50
#define MAX_ANAK 10

typedef struct {
    char deskripsi[100];
    int hari, bulan, tahun;
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

// ------------------- FUNGSI TAMBAHAN ------------------- //
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

void simpanKeFile(Tugas t) {
    FILE* file = fopen("tugas.txt", "a");
    if (file) {
        fprintf(file, "%s;%02d;%02d;%04d;%d\n", t.deskripsi, t.hari, t.bulan, t.tahun, t.selesai);
        fclose(file);
    } else {
        printf("Gagal menyimpan ke file!\n");
    }
}

// ------------------- STACK & QUEUE ------------------- //
void pushUndo(Tugas t) {
    if (top < MAX_STACK - 1) {
        stackUndo[++top] = t;
    }
}
Tugas popUndo() {
    if (top >= 0) return stackUndo[top--];
    Tugas kosong = {"", 0, 0, 0};
    return kosong;
}
void enqueue(Tugas t) {
    if (countQueue < MAX_TUGAS) {
        rear = (rear + 1) % MAX_TUGAS;
        queueTugas[rear] = t;
        countQueue++;
    }
}
Tugas dequeue() {
    Tugas t = {"", 0, 0, 0};
    if (countQueue > 0) {
        t = queueTugas[front];
        front = (front + 1) % MAX_TUGAS;
        countQueue--;
    }
    return t;
}

// ------------------- SORT & SEARCH ------------------- //
void sortTugas() {
    for (int i = 0; i < jumlahTugas - 1; i++) {
        for (int j = 0; j < jumlahTugas - i - 1; j++) {
            if (daftarTugas[j].tahun > daftarTugas[j + 1].tahun ||
                (daftarTugas[j].tahun == daftarTugas[j + 1].tahun && daftarTugas[j].bulan > daftarTugas[j + 1].bulan) ||
                (daftarTugas[j].tahun == daftarTugas[j + 1].tahun && daftarTugas[j].bulan == daftarTugas[j + 1].bulan &&
                 daftarTugas[j].hari > daftarTugas[j + 1].hari)) {
                Tugas temp = daftarTugas[j];
                daftarTugas[j] = daftarTugas[j + 1];
                daftarTugas[j + 1] = temp;
            }
        }
    }
}
int cariTugas(const char* keyword) {
    for (int i = 0; i < jumlahTugas; i++) {
        if (strstr(daftarTugas[i].deskripsi, keyword)) {
            return i;
        }
    }
    return -1;
}

// ------------------- TREE ------------------- //
TreeNode* buatNode(Tugas data) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->data = data;
    node->jumlahAnak = 0;
    return node;
}
void tambahSubTugas(TreeNode* parent, Tugas data) {
    if (parent->jumlahAnak < MAX_ANAK) {
        parent->anak[parent->jumlahAnak++] = buatNode(data);
    }
}
TreeNode* cariNode(TreeNode* node, const char* keyword) {
    if (!node) return NULL;
    if (strstr(node->data.deskripsi, keyword)) return node;
    for (int i = 0; i < node->jumlahAnak; i++) {
        TreeNode* hasil = cariNode(node->anak[i], keyword);
        if (hasil) return hasil;
    }
    return NULL;
}
void tampilTree(TreeNode* node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth; i++) printf("--");
    printf("%s (Deadline: %02d/%02d/%04d)\n", node->data.deskripsi, node->data.hari, node->data.bulan, node->data.tahun);
    for (int i = 0; i < node->jumlahAnak; i++) {
        tampilTree(node->anak[i], depth + 1);
    }
}

// ------------------- MENU ------------------- //
void tambahTugas() {
    if (jumlahTugas >= MAX_TUGAS) {
        printf("Daftar penuh!\n");
        return;
    }

    getchar();
    Tugas t;
    printf("Deskripsi: ");
    fgets(t.deskripsi, sizeof(t.deskripsi), stdin);
    t.deskripsi[strcspn(t.deskripsi, "\n")] = 0;

    printf("Deadline (DD MM YYYY): ");
    scanf("%d %d %d", &t.hari, &t.bulan, &t.tahun);
    t.selesai = 0;

    daftarTugas[jumlahTugas++] = t;
    enqueue(t);
    pushUndo(t);
    simpanKeFile(t);

    printf("Tugas berhasil ditambah!\n");
}

void tampilTugas() {
    sortTugas();
    printf("\n=== Daftar Tugas (Urut Deadline) ===\n");
    for (int i = 0; i < jumlahTugas; i++) {
        int selisih = hitungSelisihHari(daftarTugas[i].hari, daftarTugas[i].bulan, daftarTugas[i].tahun);
        char statusWaktu[50];
        if (selisih > 0)
            sprintf(statusWaktu, "lagi %d hari", selisih);
        else if (selisih == 0)
            sprintf(statusWaktu, "hari ini!");
        else
            sprintf(statusWaktu, "terlewat %d hari", -selisih);

        printf("%d. %s (Deadline: %02d/%02d/%04d) [%s] - %s\n",
               i + 1, daftarTugas[i].deskripsi,
               daftarTugas[i].hari, daftarTugas[i].bulan, daftarTugas[i].tahun,
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
        printf("Tugas ditemukan: %s (Deadline: %02d/%02d/%04d) [%s]\n",
               daftarTugas[idx].deskripsi,
               daftarTugas[idx].hari,
               daftarTugas[idx].bulan,
               daftarTugas[idx].tahun,
               daftarTugas[idx].selesai ? "Selesai" : "Belum");
    } else {
        printf("Tugas tidak ditemukan.\n");
    }
}

void menuTree() {
    int pilihan;
    do {
        printf("\n=== Menu Struktur Tugas Bertingkat (Tree) ===\n");
        printf("1. Buat Root Tugas\n");
        printf("2. Tambah Sub-Tugas\n");
        printf("3. Tampilkan Struktur\n");
        printf("4. Kembali\n");
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
            printf("Deadline root (DD MM YYYY): ");
            scanf("%d %d %d", &t.hari, &t.bulan, &t.tahun);
            t.selesai = 0;
            akar = buatNode(t);
            printf("Root berhasil dibuat!\n");
        } else if (pilihan == 2) {
            if (!akar) {
                printf("Buat root terlebih dahulu!\n");
                continue;
            }
            getchar();
            char keyword[100];
            printf("Masukkan kata kunci parent: ");
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
            printf("Deadline sub-tugas (DD MM YYYY): ");
            scanf("%d %d %d", &sub.hari, &sub.bulan, &sub.tahun);
            sub.selesai = 0;

            tambahSubTugas(parent, sub);
            printf("Sub-tugas berhasil ditambahkan!\n");
        } else if (pilihan == 3) {
            if (!akar) {
                printf("Belum ada struktur tugas.\n");
            } else {
                tampilTree(akar, 0);
            }
        }
    } while (pilihan != 4);
}

int main() {
    int pilihan;
    do {
        printf("\n=== Menu Aplikasi ===\n");
        printf("1. Tambah Tugas\n");
        printf("2. Tampil Daftar Tugas\n");
        printf("3. Undo Tambah Tugas (Stack)\n");
        printf("4. Tampil Antrian Tugas (Queue)\n");
        printf("5. Cari Tugas (Searching)\n");
        printf("6. Menu Struktur Tugas Bertingkat (Tree)\n");
        printf("7. Tandai Tugas Selesai\n");
        printf("8. Keluar\n");
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
            default: printf("Pilihan tidak valid.\n"); break;
        }
    } while (pilihan != 8);

    return 0;
}