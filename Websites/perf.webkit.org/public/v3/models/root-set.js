
class RootSet extends DataModelObject {

    constructor(id, object)
    {
        super(id);
        this._repositories = [];
        this._repositoryToCommitMap = {};
        this._latestCommitTime = null;

        if (!object)
            return;

        for (var row of object.roots) {
            var repositoryId = row.repository;
            var repository = Repository.findById(repositoryId);

            console.assert(!this._repositoryToCommitMap[repositoryId]);
            this._repositoryToCommitMap[repositoryId] = CommitLog.ensureSingleton(repository, row);
            this._repositories.push(repository);
        }
    }

    repositories() { return this._repositories; }
    commitForRepository(repository) { return this._repositoryToCommitMap[repository.id()]; }

    latestCommitTime()
    {
        if (this._latestCommitTime == null) {
            var maxTime = 0;
            for (var repositoryId in this._repositoryToCommitMap)
                maxTime = Math.max(maxTime, +this._repositoryToCommitMap[repositoryId].time());
            this._latestCommitTime = maxTime;
        }
        return this._latestCommitTime;
    }

    equals(other)
    {
        if (this._repositories.length != other._repositories.length)
            return false;
        for (var repositoryId in this._repositoryToCommitMap) {
            if (this._repositoryToCommitMap[repositoryId] != other._repositoryToCommitMap[repositoryId])
                return false;
        }
        return true;
    }

    static containsMultipleCommitsForRepository(rootSets, repository)
    {
        console.assert(repository instanceof Repository);
        if (rootSets.length < 2)
            return false;
        var firstCommit = rootSets[0].commitForRepository(repository);
        for (var set of rootSets) {
            var anotherCommit = set.commitForRepository(repository);
            if (!firstCommit != !anotherCommit || (firstCommit && firstCommit.revision() != anotherCommit.revision()))
                return true;
        }
        return false;
    }
}

class MeasurementRootSet extends RootSet {

    constructor(id, revisionList)
    {
        super(id, null);
        for (var values of revisionList) {
            var repositoryId = values[0];
            var repository = Repository.findById(repositoryId);

            this._repositoryToCommitMap[repositoryId] = CommitLog.ensureSingleton(repository, {revision: values[1], time: values[2]});
            this._repositories.push(repository);
        }
    }

    static ensureSingleton(measurementId, revisionList)
    {
        var rootSetId = measurementId + '-rootset';
        return RootSet.findById(rootSetId) || (new MeasurementRootSet(rootSetId, revisionList));
    }
}
