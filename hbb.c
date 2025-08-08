#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkentry.h>
#include <string.h>
#include <curl/curl.h>

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	
	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if(ptr == NULL) {
		printf("Runtime error fetching content: out of memory\n");
		return 0;
	}
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}

char *fetchURL(const char *geturl) {
	CURL *curl_handle;
	CURLcode res;
	
	struct MemoryStruct chunk;
	
	chunk.memory = malloc(1);
	chunk.size = 0;
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	
	if(curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_URL, geturl);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "HomeBrewBrowser/I0.0.1"); // probably make this a little better
		res = curl_easy_perform(curl_handle);
		
		if(res != CURLE_OK) {
			fprintf(stderr, "Fetch failed: %s\n", curl_easy_strerror(res));
		} else {
			printf("%lu bytes retrieved from %s\n", (unsigned long)chunk.size, geturl);
		}
		
		curl_easy_cleanup(curl_handle);
		return chunk.memory;
	}
}

typedef struct {
	char *tag;
	int end;
} Tag;
//Tag findEndTag(const char *data, const int start, const char *target) {
//	data += start;
//	char *tag = malloc(256); 
//	if (!tag) return (Tag){NULL, 0};
//	Tag tags = {NULL, 0};
//	while (1) {
//		int i = 0;
//		while (*data != '<' && *data != '\0') {
//			if (*data == '\0') break;
//			data++;
//		} if (*data == '\0') break; data++;
//		while (*data != '>' && i < 255) {
//			if (*data == '\0') break;
//			tag[i++] = *data++;
//		} if (*data == '\0') break; data++;
//		if (strcmp(tag, target) == 0) {
//			break;
//		} else {
//			tag[1] = '\0'
//			continue;
//		}
//	}
//	tag[i] = '\0';
//	i += 2;
//	printf("got tag %s that ends at %i\n", tag, i);
//	tags.tag = tag;
//	tags.end = i;
//	return tags;
//}           WORKING ON THIS
Tag findFirstTag(const char *data, const int start) {
	data += start;
	char *tag = malloc(256);
	if (!tag) return (Tag){NULL, 0};
	int i = 0;
	Tag tags = {NULL, 0};
	while (*data != '<' && *data != '\0') {
		data++;
	} data++;
	while (*data != '>' && i < 255) {
		tag[i++] = *data++;
	} data++;
	tag[i] = '\0';
	i += 2;
	printf("got tag %s that ends at %i\n", tag, i);
	tags.tag = tag;
	tags.end = i;
	return tags;
}

char *parseHTML(const char *data) {
//	while (*data != '\0') {
		findFirstTag(data, 0);
//	}
	return data; // This line completely skips this function - the rest of it is in testing
}

void on_url_submit(GtkEntry *entry, gpointer user_data) {
    GtkTextBuffer *buffer = GTK_TEXT_BUFFER(user_data);
    const char *url = gtk_editable_get_text(GTK_EDITABLE(entry));
    char *rawcontent = fetchURL(url);
    char *content = parseHTML(rawcontent);

    if (content) {
        gtk_text_buffer_set_text(buffer, content, -1);
        free(content);
    } else {
        gtk_text_buffer_set_text(buffer, "[Failed to fetch data]", -1);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
	printf("Launching HBB...\n");
	GtkWidget *window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Homebrew Browser (I0.0.1)");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_window_set_child(GTK_WINDOW(window), vbox);
	GtkWidget *entry = gtk_entry_new();
	gtk_box_append(GTK_BOX(vbox), entry);
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_append(GTK_BOX(vbox), scrolled);
	GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
	GtkWidget *textview = gtk_text_view_new_with_buffer(buffer);
	gtk_widget_set_vexpand(vbox, TRUE);
	gtk_widget_set_vexpand(scrolled, TRUE);
	gtk_widget_set_vexpand(textview, TRUE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview), FALSE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), textview);
	g_signal_connect(entry, "activate", G_CALLBACK(on_url_submit), buffer);
	gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char **argv) {
	GtkApplication *app = gtk_application_new("org.eightyhd.homebrewbrowser", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
