from data_ingestion import ArxivCrawler, PatentCrawler, SemanticScholarCrawler, NasaCrawler


def test_arxiv_crawler_fallback():
    r = ArxivCrawler().search("finite element", limit=1)
    assert len(r) >= 1


def test_patent_crawler():
    r = PatentCrawler().search_patents("bearing", limit=1)
    assert len(r) >= 1
