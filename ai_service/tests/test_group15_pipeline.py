from ai.pipeline_engine import get_pipeline_engine


def test_pipeline_run():
    eng = get_pipeline_engine()
    job = eng.run_pipeline("")
    assert job.job_id
