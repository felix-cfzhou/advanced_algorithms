#include "splay_tree.h"

int main() {
    splay_tree<int, int> st;
    
    st.insert(6, 6);
    st.print();

    st.insert(4, 7);
    st.print();

    st.insert(7, 8);
    st.print();

    st.insert(6, 16);
    st.print();

    st.insert(4, 17);
    st.print();

    st.insert(7, 18);
    st.print();

    for(int k=9; k<30; ++k) st.insert(k % 2 == 0 ? k : -k, k);
    st.print();
}
