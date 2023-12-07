namespace tcms {
    struct Content {

    };

    class Article {
    private:
        int id;
    public:
        Article(int id);
        ~Article();
        Content get_content();
    };

}
