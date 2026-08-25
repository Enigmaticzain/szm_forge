import kg_rdf


def test_rdf_ingest_or_skip():
    if kg_rdf.available():
        r = kg_rdf.ingest_document_triples("doc1", "Test", ["bearing", "steel"])
        assert r["status"] == "success"
    else:
        r = kg_rdf.ingest_document_triples("doc1", "Test", [])
        assert r["status"] == "error"
