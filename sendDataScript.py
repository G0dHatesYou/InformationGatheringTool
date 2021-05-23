from py2neo import Graph
import requests
from bs4 import BeautifulSoup
from bs4.element import Comment
import argparse
import sys


def tag_visible(element):
    if element.parent.name in ['style', 'script', 'head', 'title', 'meta', '[document]']:
        return False
    if isinstance(element, Comment):
        return False
    return True


def text_from_html(page):
    soup = BeautifulSoup(page.content, 'html.parser')
    texts = soup.findAll(text=True)
    visible_texts = filter(tag_visible, texts)
    return u" ".join(t.strip() for t in visible_texts)


def remove_copy(links):
    seen = set()
    result = []
    for item in links:
        if item not in seen:
            seen.add(item)
            result.append(item)
    return result

if __name__ == '__main__':
    print("wtf")
    parser = argparse.ArgumentParser()

    parser.add_argument('-url', action='store', dest='URL',
                        help='specify URL to scrap')
    arguments = parser.parse_args()

    if arguments.URL:
        URL = arguments.URL
    else:
        parser.print_help()
        sys.exit(1)

    page = requests.get(URL)

    soup = BeautifulSoup(page.content, 'html.parser')
    raw_links = soup.find_all('a')

    link_set = set()
    link_set.add(URL)
    for tag in raw_links:
        link = tag.get('href', None)
        if link is not None and link.startswith('https'):
            link_set.add(link)

    links = remove_copy(link_set)

    for link in link_set:
        print(link)
        page = requests.get(link)
        cleanText = text_from_html(page).replace("'", ".").replace("$", " ").replace("\\", " ").replace("//", " ")
        graph = Graph("http://localhost:7474", auth=("neo4j", "klkl"))
        result = graph.run("CREATE (website:Website { title:'website%s', content:'%s' })" % (link, cleanText))
        print('create result ', result)
        result = graph.run("MATCH (a:Website), (b:Website) WHERE a.title = 'website%s' AND b.title = 'website%s' CREATE(a) - [: MENTIONED]->(b)" % (URL, link))
        print('create relationship ', result)
        #MATCH(a: Website), (m:Website) OPTIONAL  MATCH(a) - [r1] - () DELETE  a, r1, m

    print(len(link_set))
