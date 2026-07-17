export type TrainingDataType = 'text' | 'pdf' | 'image' | 'video' | 'audio';

export interface TrainingData {
  id: string;
  type: TrainingDataType;
  content: string;
  extractedText: string;
  fileName?: string;
  mimeType?: string;
  timestamp: Date;
  wordCount: number;
}

export interface LocalModelTerm {
  term: string;
  count: number;
  score: number;
}

export interface LocalModelPhrase {
  phrase: string;
  count: number;
  score: number;
}

export interface LocalModelDocument {
  id: string;
  sourceName: string;
  type: TrainingDataType;
  preview: string;
  wordCount: number;
  sentenceCount: number;
  keyTerms: LocalModelTerm[];
  representativeSentence: string;
  timestamp: Date;
}

export interface LocalModelCitation {
  sourceName: string;
  excerpt: string;
  score: number;
}

interface LocalModelChunk extends LocalModelCitation {
  id: string;
  documentId: string;
  type: TrainingDataType;
  tokenCount: number;
  tokens: string[];
  frequencies: Record<string, number>;
}

export interface LocalLanguageModel {
  documents: LocalModelDocument[];
  chunks: LocalModelChunk[];
  totalDocuments: number;
  totalWords: number;
  totalSentences: number;
  totalCharacters: number;
  vocabularySize: number;
  averageWordsPerDocument: number;
  topTerms: LocalModelTerm[];
  topPhrases: LocalModelPhrase[];
  keySentences: string[];
  corpusPreview: string;
  documentTypeBreakdown: Record<TrainingDataType, number>;
}

export interface LocalModelAnswer {
  success: boolean;
  answer: string;
  matchedTerms: string[];
  citations: LocalModelCitation[];
  confidence: number;
}

const STOP_WORDS = new Set([
  'a',
  'about',
  'above',
  'after',
  'again',
  'against',
  'all',
  'am',
  'an',
  'and',
  'any',
  'are',
  'as',
  'at',
  'be',
  'because',
  'been',
  'before',
  'being',
  'below',
  'between',
  'both',
  'but',
  'by',
  'can',
  'could',
  'did',
  'do',
  'does',
  'doing',
  'down',
  'during',
  'each',
  'few',
  'for',
  'from',
  'further',
  'had',
  'has',
  'have',
  'having',
  'he',
  'her',
  'here',
  'hers',
  'herself',
  'him',
  'himself',
  'his',
  'how',
  'i',
  'if',
  'in',
  'into',
  'is',
  'it',
  'its',
  'itself',
  'just',
  'me',
  'more',
  'most',
  'my',
  'myself',
  'no',
  'nor',
  'not',
  'of',
  'off',
  'on',
  'once',
  'only',
  'or',
  'other',
  'our',
  'ours',
  'ourselves',
  'out',
  'over',
  'own',
  'same',
  'she',
  'should',
  'so',
  'some',
  'such',
  'than',
  'that',
  'the',
  'their',
  'theirs',
  'them',
  'themselves',
  'then',
  'there',
  'these',
  'they',
  'this',
  'those',
  'through',
  'to',
  'too',
  'under',
  'until',
  'up',
  'very',
  'was',
  'we',
  'were',
  'what',
  'when',
  'where',
  'which',
  'while',
  'who',
  'whom',
  'why',
  'will',
  'with',
  'would',
  'you',
  'your',
  'yours',
  'yourself',
  'yourselves',
]);

const EMPTY_BREAKDOWN: Record<TrainingDataType, number> = {
  text: 0,
  pdf: 0,
  image: 0,
  video: 0,
  audio: 0,
};

function normalizeWhitespace(text: string) {
  return text.replace(/\r\n/g, '\n').replace(/[ \t]+/g, ' ').replace(/\n{3,}/g, '\n\n').trim();
}

function countWords(text: string) {
  const matches = text.match(/\b[\p{L}\p{N}'-]+\b/gu);
  return matches?.length ?? 0;
}

function tokenize(text: string, options?: { includeStopWords?: boolean }) {
  const includeStopWords = options?.includeStopWords ?? false;
  const matches = text
    .toLowerCase()
    .match(/\b[\p{L}\p{N}][\p{L}\p{N}'-]*\b/gu);

  if (!matches) {
    return [];
  }

  return matches.filter((token) => {
    if (token.length < 2) {
      return false;
    }

    if (!includeStopWords && STOP_WORDS.has(token)) {
      return false;
    }

    return true;
  });
}

function countFrequencies(tokens: string[]) {
  return tokens.reduce<Record<string, number>>((acc, token) => {
    acc[token] = (acc[token] ?? 0) + 1;
    return acc;
  }, {});
}

function splitSentences(text: string) {
  return normalizeWhitespace(text)
    .split(/(?<=[.!?])\s+|\n+/)
    .map((sentence) => sentence.trim())
    .filter((sentence) => sentence.length > 0);
}

function createPreview(text: string, limit = 180) {
  if (text.length <= limit) {
    return text;
  }

  return `${text.slice(0, limit).trimEnd()}...`;
}

function makeSourceName(document: TrainingData, index: number) {
  return document.fileName?.trim() || `Training note ${index + 1}`;
}

function chunkSentences(sentences: string[], maxWords = 90, maxSentences = 3) {
  if (sentences.length === 0) {
    return [];
  }

  const chunks: string[] = [];
  let buffer: string[] = [];
  let wordTotal = 0;

  for (const sentence of sentences) {
    const wordCountForSentence = countWords(sentence);
    const shouldFlush =
      buffer.length > 0 &&
      (buffer.length >= maxSentences || wordTotal + wordCountForSentence > maxWords);

    if (shouldFlush) {
      chunks.push(buffer.join(' '));
      buffer = [];
      wordTotal = 0;
    }

    buffer.push(sentence);
    wordTotal += wordCountForSentence;
  }

  if (buffer.length > 0) {
    chunks.push(buffer.join(' '));
  }

  return chunks;
}

function scoreTermsFromFrequencies(
  frequencies: Record<string, number>,
  documentFrequencies: Record<string, number>,
  totalUnits: number
) {
  const scoredTerms = Object.entries(frequencies).map(([term, count]) => {
    const df = documentFrequencies[term] ?? 1;
    const score = count * (Math.log((totalUnits + 1) / df) + 1);

    return {
      term,
      count,
      score,
    };
  });

  scoredTerms.sort((left, right) => right.score - left.score || right.count - left.count);
  return scoredTerms;
}

function buildPhraseScores(documents: TrainingData[]) {
  const phraseCounts: Record<string, number> = {};

  for (const document of documents) {
    const tokens = tokenize(document.extractedText);
    for (let index = 0; index < tokens.length - 1; index += 1) {
      const first = tokens[index];
      const second = tokens[index + 1];

      if (STOP_WORDS.has(first) || STOP_WORDS.has(second)) {
        continue;
      }

      const phrase = `${first} ${second}`;
      phraseCounts[phrase] = (phraseCounts[phrase] ?? 0) + 1;
    }
  }

  return Object.entries(phraseCounts)
    .map(([phrase, count]) => ({
      phrase,
      count,
      score: count * phrase.split(' ').length,
    }))
    .sort((left, right) => right.score - left.score || right.count - left.count)
    .slice(0, 12);
}

function scoreSentence(sentence: string, termScores: Record<string, number>) {
  const tokens = tokenize(sentence);
  if (tokens.length === 0) {
    return 0;
  }

  const total = tokens.reduce((sum, token) => sum + (termScores[token] ?? 0), 0);
  return total / Math.sqrt(tokens.length);
}

function createOverview(model: LocalLanguageModel) {
  if (model.totalDocuments === 0) {
    return 'No training data has been loaded yet.';
  }

  const topConcepts = model.topTerms.slice(0, 4).map((term) => term.term);
  const conceptsLine =
    topConcepts.length > 0 ? ` Main concepts: ${topConcepts.join(', ')}.` : '';

  return `The local model has learned from ${model.totalDocuments} document${
    model.totalDocuments === 1 ? '' : 's'
  }, covering ${model.totalWords.toLocaleString()} words and ${model.vocabularySize.toLocaleString()} unique terms.${conceptsLine}`;
}

export function createTrainingEntry(input: {
  type: TrainingDataType;
  content: string;
  extractedText?: string;
  fileName?: string;
  mimeType?: string;
  timestamp?: Date;
}) {
  const timestamp = input.timestamp ?? new Date();
  const extractedText = normalizeWhitespace(input.extractedText ?? input.content);

  return {
    id: `${timestamp.getTime()}-${Math.random().toString(36).slice(2, 8)}`,
    type: input.type,
    content: input.content,
    extractedText,
    fileName: input.fileName,
    mimeType: input.mimeType,
    timestamp,
    wordCount: countWords(extractedText),
  } satisfies TrainingData;
}

export function buildLocalLanguageModel(trainingHistory: TrainingData[]): LocalLanguageModel {
  if (trainingHistory.length === 0) {
    return {
      documents: [],
      chunks: [],
      totalDocuments: 0,
      totalWords: 0,
      totalSentences: 0,
      totalCharacters: 0,
      vocabularySize: 0,
      averageWordsPerDocument: 0,
      topTerms: [],
      topPhrases: [],
      keySentences: [],
      corpusPreview: '',
      documentTypeBreakdown: { ...EMPTY_BREAKDOWN },
    };
  }

  const documentTypeBreakdown = { ...EMPTY_BREAKDOWN };
  const rawDocuments = trainingHistory.map((document, index) => {
    const sourceName = makeSourceName(document, index);
    const text = normalizeWhitespace(document.extractedText);
    const sentences = splitSentences(text);
    const tokens = tokenize(text);
    const termCounts = countFrequencies(tokens);

    documentTypeBreakdown[document.type] += 1;

    return {
      sourceName,
      text,
      sentences,
      tokens,
      termCounts,
      document,
    };
  });

  const chunks = rawDocuments.flatMap((item) => {
    const sentenceChunks = chunkSentences(item.sentences);
    const chunkBodies = sentenceChunks.length > 0 ? sentenceChunks : [item.text];

    return chunkBodies
      .map((chunkText, chunkIndex) => {
        const tokens = tokenize(chunkText);
        if (tokens.length === 0) {
          return null;
        }

        return {
          id: `${item.document.id}-chunk-${chunkIndex + 1}`,
          documentId: item.document.id,
          sourceName: item.sourceName,
          excerpt: chunkText,
          score: 0,
          type: item.document.type,
          tokenCount: tokens.length,
          tokens,
          frequencies: countFrequencies(tokens),
        } satisfies LocalModelChunk;
      })
      .filter((chunk): chunk is LocalModelChunk => chunk !== null);
  });

  const chunkDocumentFrequencies = chunks.reduce<Record<string, number>>((acc, chunk) => {
    const uniqueTerms = new Set(chunk.tokens);
    for (const token of uniqueTerms) {
      acc[token] = (acc[token] ?? 0) + 1;
    }
    return acc;
  }, {});

  const documentFrequencies = rawDocuments.reduce<Record<string, number>>((acc, item) => {
    const uniqueTerms = new Set(item.tokens);
    for (const token of uniqueTerms) {
      acc[token] = (acc[token] ?? 0) + 1;
    }
    return acc;
  }, {});

  const aggregateTermCounts = rawDocuments.reduce<Record<string, number>>((acc, item) => {
    for (const [term, count] of Object.entries(item.termCounts)) {
      acc[term] = (acc[term] ?? 0) + count;
    }
    return acc;
  }, {});

  const topTerms = scoreTermsFromFrequencies(
    aggregateTermCounts,
    documentFrequencies,
    Math.max(rawDocuments.length, 1)
  ).slice(0, 16);

  const termScoreLookup = topTerms.reduce<Record<string, number>>((acc, item) => {
    acc[item.term] = item.score;
    return acc;
  }, {});

  const documents = rawDocuments.map((item) => {
    const scoredTerms = scoreTermsFromFrequencies(
      item.termCounts,
      documentFrequencies,
      Math.max(rawDocuments.length, 1)
    ).slice(0, 6);
    const representativeSentence =
      [...item.sentences]
        .sort(
          (left, right) =>
            scoreSentence(right, termScoreLookup) - scoreSentence(left, termScoreLookup)
        )[0] || createPreview(item.text, 140);

    return {
      id: item.document.id,
      sourceName: item.sourceName,
      type: item.document.type,
      preview: createPreview(item.text),
      wordCount: item.document.wordCount,
      sentenceCount: item.sentences.length,
      keyTerms: scoredTerms,
      representativeSentence,
      timestamp: item.document.timestamp,
    } satisfies LocalModelDocument;
  });

  const topPhrases = buildPhraseScores(trainingHistory);

  const keySentences = rawDocuments
    .flatMap((item) => item.sentences)
    .map((sentence) => ({
      sentence,
      score: scoreSentence(sentence, termScoreLookup),
    }))
    .filter((item) => item.score > 0)
    .sort((left, right) => right.score - left.score)
    .slice(0, 5)
    .map((item) => item.sentence);

  const totalWords = trainingHistory.reduce((sum, item) => sum + item.wordCount, 0);
  const totalCharacters = trainingHistory.reduce((sum, item) => sum + item.extractedText.length, 0);
  const totalSentences = rawDocuments.reduce((sum, item) => sum + item.sentences.length, 0);
  const vocabularySize = Object.keys(aggregateTermCounts).length;

  return {
    documents,
    chunks,
    totalDocuments: trainingHistory.length,
    totalWords,
    totalSentences,
    totalCharacters,
    vocabularySize,
    averageWordsPerDocument: totalWords / Math.max(trainingHistory.length, 1),
    topTerms,
    topPhrases,
    keySentences,
    corpusPreview: createPreview(rawDocuments[0]?.text ?? '', 320),
    documentTypeBreakdown,
  };
}

function selectEvidenceSentences(chunks: LocalModelChunk[], queryTerms: string[]) {
  const rankedSentences = chunks.flatMap((chunk) =>
    splitSentences(chunk.excerpt).map((sentence) => {
      const tokens = tokenize(sentence);
      const overlap = queryTerms.filter((term) => tokens.includes(term)).length;
      return {
        sentence,
        overlap,
        score: chunk.score + overlap,
      };
    })
  );

  return rankedSentences
    .filter((item) => item.overlap > 0)
    .sort((left, right) => right.score - left.score)
    .slice(0, 3)
    .map((item) => item.sentence);
}

export function answerWithLocalModel(model: LocalLanguageModel, prompt: string): LocalModelAnswer {
  if (model.totalDocuments === 0) {
    return {
      success: false,
      answer: 'No training data is loaded yet. Paste English text or import documents in the Train tab first.',
      matchedTerms: [],
      citations: [],
      confidence: 0,
    };
  }

  const trimmedPrompt = prompt.trim();
  const normalizedPrompt = trimmedPrompt.toLowerCase();

  if (/^(hi|hello|hey)\b/.test(normalizedPrompt)) {
    return {
      success: true,
      answer: `${createOverview(model)} Ask me about any topic that appears in your training documents.`,
      matchedTerms: [],
      citations: [],
      confidence: 0.92,
    };
  }

  if (/(what did you learn|what have you learned|summary|overview|status)/.test(normalizedPrompt)) {
    const evidence = model.keySentences.slice(0, 2).join(' ');
    return {
      success: true,
      answer: `${createOverview(model)} ${evidence}`.trim(),
      matchedTerms: model.topTerms.slice(0, 5).map((item) => item.term),
      citations: [],
      confidence: 0.88,
    };
  }

  const queryTerms = tokenize(trimmedPrompt);

  if (queryTerms.length === 0) {
    return {
      success: true,
      answer: `${createOverview(model)} Try asking about a keyword, component, process, or sentence from your documents.`,
      matchedTerms: [],
      citations: [],
      confidence: 0.65,
    };
  }

  const rankedChunks = model.chunks
    .map((chunk) => {
      const score = queryTerms.reduce((sum, term) => {
        const frequency = chunk.frequencies[term] ?? 0;
        return sum + frequency;
      }, 0);

      return {
        ...chunk,
        score: score / Math.sqrt(chunk.tokenCount || 1),
      };
    })
    .filter((chunk) => chunk.score > 0)
    .sort((left, right) => right.score - left.score)
    .slice(0, 3);

  if (rankedChunks.length === 0) {
    return {
      success: false,
      answer: `I could not find direct evidence for "${trimmedPrompt}" in the trained documents yet. Try using words that appear in your sources, or add more text to the model.`,
      matchedTerms: [],
      citations: [],
      confidence: 0.12,
    };
  }

  const evidenceSentences = selectEvidenceSentences(rankedChunks, queryTerms);
  const answerText =
    evidenceSentences.length > 0
      ? evidenceSentences.join(' ')
      : rankedChunks.map((chunk) => chunk.excerpt).join(' ');

  const matchedTerms = queryTerms.filter((term, index) => queryTerms.indexOf(term) === index);

  return {
    success: true,
    answer: answerText,
    matchedTerms,
    citations: rankedChunks.map(({ sourceName, excerpt, score }) => ({
      sourceName,
      excerpt: createPreview(excerpt, 220),
      score,
    })),
    confidence: Math.min(0.98, 0.35 + rankedChunks[0].score / 3),
  };
}

export function getLocalModelStatus(model: LocalLanguageModel) {
  if (model.totalDocuments === 0) {
    return 'Local model is empty. Load training data to begin learning.';
  }

  const topTerms = model.topTerms.slice(0, 6).map((item) => item.term).join(', ');

  return [
    `Documents: ${model.totalDocuments}`,
    `Words learned: ${model.totalWords.toLocaleString()}`,
    `Vocabulary: ${model.vocabularySize.toLocaleString()} unique terms`,
    `Sentences indexed: ${model.totalSentences.toLocaleString()}`,
    `Top concepts: ${topTerms || 'No concepts extracted yet'}`,
  ].join('\n');
}

export function loadTrainingHistoryFromStorage(storageKey: string) {
  if (typeof window === 'undefined') {
    return [] as TrainingData[];
  }

  try {
    const rawValue = window.localStorage.getItem(storageKey);
    if (!rawValue) {
      return [] as TrainingData[];
    }

    const parsed = JSON.parse(rawValue) as Array<Omit<TrainingData, 'timestamp'> & { timestamp: string }>;
    return parsed.map((item) => ({
      ...item,
      timestamp: new Date(item.timestamp),
    }));
  } catch {
    return [] as TrainingData[];
  }
}
